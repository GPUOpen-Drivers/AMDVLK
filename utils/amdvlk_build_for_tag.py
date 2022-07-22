#!/usr/bin/python3

# This script is used to build the AMD open source vulkan driver and make a deb package from github for tags.

# Before running this script, please install dependency packages with
# pip3 install gitpython

import sys
import os
import string
import time
import datetime
import git
import shutil
import re
from optparse import OptionParser
import xml.etree.ElementTree as ET

class Worker:
    def __init__(self):
        self.workDir      = os.getcwd()
        self.srcDir       = self.workDir + '/amdvlk_src/'
        self.pkgDir       = self.workDir + '/amdvlk_pkg/'
        self.buildDir     = ''
        self.pkgSharedDir = os.path.join(self.workDir, 'pkgShared')

        self.descript     = ''
        self.targetRepo   = 'https://github.com/GPUOpen-Drivers/'
        self.distro       = self.DistributionType()

        self.buildTag     = ''
        self.driverRoot   = ''
        self.repoCommits  = {}
        self.repoPathes   = {}
        self.diffTag      = 'v-2022.Q3.1'


    def GetOpt(self):
        parser = OptionParser()

        parser.add_option("-w", "--workDir", action="store",
                          type="string",
                          dest="workDir",
                          help="Specify the location of source code, or download it from github")

        parser.add_option("-t", "--targetRepo", action="store",
                          type="string",
                          dest="targetRepo",
                          help="Specify the target repo of github, default is " + self.targetRepo)

        parser.add_option("-b", "--buildTag", action="store",
                          type="string",
                          dest="buildTag",
                          help="Specify the tag to build, default is latest")

        (options, args) = parser.parse_args()

        if options.workDir:
            print("The source code is under %s" % (options.workDir))
            self.workDir = options.workDir
            self.srcDir  = self.workDir + "/amdvlk_src/"
            self.pkgDir  = self.workDir + "/amdvlk_pkg/"
        else:
            print("The source code is not specified, downloading from github to: " + self.workDir)

        if not os.path.exists(self.srcDir):
            os.makedirs(self.srcDir)

        if options.targetRepo:
            self.targetRepo = options.targetRepo

        print("The target repo is " + self.targetRepo)

        if options.buildTag:
            self.buildTag = options.buildTag

    def DistributionType(self):
        result = os.popen('lsb_release -is').read().strip()
        if (result == 'Ubuntu'):
            return result
        elif (result == 'RedHatEnterprise' or result == 'RedHatEnterpriseWorkstation'):
            return 'RHEL'
        else:
            print('Unknown Linux distribution: ' + result)
            sys.exit(-1)

    def IsBuildTagNewer(self):
        return self.buildTag > self.diffTag

    def SyncAMDVLK(self):
        # Sync all amdvlk repoes and checkout AMDVLK to specified tag
        os.chdir(self.srcDir)

        # TODO: we use the latest build_with_tools.xml to init the repo,
        # please update the command when the manifest is changed
        repoName = 'AMDVLK'
        initcmd='repo init -u ' + self.targetRepo + repoName + ' -b master -m build_with_tools.xml'
        if os.system(initcmd):
            print(initcmd + ' failed')
            exit(-1)
        if os.system('repo sync -j8'):
            print('repo sync failed')
            exit(-1)

        # Simply use repo command instead of reading from manifest to get path
        amdvlkPath = os.popen('repo list --path-only ' + repoName).read().strip()
        self.driverRoot = self.srcDir + amdvlkPath[:-len(repoName)]
        repo = git.Repo(amdvlkPath)
        if not self.buildTag: self.buildTag = repo.tags[-1].name
        validTag = False
        for tagRef in repo.tags:
            if self.buildTag == tagRef.name:
                validTag = True
                if self.IsBuildTagNewer():
                    self.descript = tagRef.tag.message
                else:
                    self.descript = tagRef.commit.message
                self.version = self.buildTag[2:]
                break

        if not validTag:
            print('Not a valid tag: ' + self.buildTag)
            sys.exit(-1)
        repo.git.checkout(self.buildTag)

    def ParseManifest(self, manifestXml):
        os.chdir(os.path.dirname(os.path.abspath(manifestXml)))
        xmlRoot = ET.parse(manifestXml).getroot()
        for child in xmlRoot:
            if child.tag == 'include':
                self.ParseManifest(child.attrib['name'])
            if child.tag == 'project' and 'path' in child.attrib:
                component = child.attrib['name']
                if component == 'AMDVLK': continue
                self.repoPathes[component] = child.attrib['path']
                self.repoCommits[component] = child.attrib['revision']

    def CheckoutDriver(self):
        os.chdir(self.srcDir)

        # Get the tagged commits of driver components from manifest
        manifestXml = self.driverRoot
        if self.IsBuildTagNewer():
            manifestXml += 'AMDVLK/build_with_tools.xml'
        else:
            manifestXml += 'AMDVLK/default.xml'
        if not os.path.exists(manifestXml):
            print('Manifest file: ' + manifestXml + ' not found!')
            sys.exit(-1)
        self.ParseManifest(manifestXml)

        # Checkout commits
        for i in self.repoCommits:
            print('Checking out ' + i + ': ' + self.repoCommits[i])
            repo = git.Repo(self.repoPathes[i])
            repo.git.clean('-xdff')
            repo.git.checkout(self.repoCommits[i])

    def GenerateReleaseNotes(self):
        os.chdir(self.workDir)
        releaseNotes = '[Driver installation instruction](' + self.targetRepo + 'AMDVLK#install-with-pre-built-driver) \n\n'
        formated_str = self.descript.replace('New feature and improvement', '## New feature and improvement')
        formated_str = formated_str.replace('Issue fix', '## Issue fix')
        releaseNotes += formated_str
        with open('amdvlk_releaseNotes.md', 'w') as notes:
            notes.write(releaseNotes + '\n')

    def Build(self):
        self.PrepareChangelog()

        if self.distro == 'Ubuntu':
            self.MakeDriverPackage('64')
            self.ArchiveAmdllpcTools('amd64')
            self.MakeDriverPackage('32')
            self.ArchiveAmdllpcTools('i386')
            self.GenerateReleaseNotes()
        elif self.distro == 'RHEL':
            self.MakeDriverPackage('64')
        print('The package is generated successfully for ' + self.buildTag)

    def MakeDriverPackage(self, arch):
        cmakeName = 'cmake '
        if (self.distro == 'RHEL'):
            cmakeName = 'source scl_source enable devtoolset-7 && cmake '

        if not self.IsBuildTagNewer():
            # Fetch spvgen resources
            os.chdir(self.driverRoot + 'spvgen/external')
            print(self.driverRoot + 'spvgen/external')
            if os.system('python3 fetch_external_sources.py'):
                print('SPVGEN: fetch external sources failed')
                exit(-1)

        self.buildDir   = 'xgl/Release64' if arch == '64' else 'xgl/Release32'
        cmakeFlags = ' -G Ninja -S xgl -B ' + self.buildDir + ' -DBUILD_WAYLAND_SUPPORT=ON -DPACKAGE_VERSION=' + self.version + ' -DXGL_BUILD_TOOLS=ON'
        cFlags     = '' if arch == '64' else ' -DCMAKE_C_FLAGS=\"-m32 -march=i686\" -DCMAKE_CXX_FLAGS=\"-m32 -march=i686\"'

        os.chdir(self.driverRoot)
        if os.path.exists(self.buildDir):
            shutil.rmtree(self.buildDir)
        os.makedirs(self.buildDir)

        # Build driver
        if os.system(cmakeName + cmakeFlags + cFlags):
            print(cmakeName + cmakeFlags + cFlags + ' failed')
            exit(-1)

        if os.system('cmake --build ' + self.buildDir):
            print('build amdvlk failed')
            exit(-1);

        # Make driver package
        if os.system('cmake --build ' + self.buildDir + ' --target makePackage'):
            print('make driver package failed')

        # Build spvgen
        if os.system('cmake --build ' + self.buildDir + ' --target spvgen'):
            print('SPVGEN: build failed')
            exit(-1);

        # Build amdllpc
        if os.system('cmake --build ' + self.buildDir + ' --target amdllpc'):
            print('build amdllpc failed')
            exit(-1);

        # Copy driver package to workDir, will be used in release step
        os.system('cp ' + self.buildDir + '/*.rpm ' + self.workDir)
        os.system('cp ' + self.buildDir + '/*.deb ' + self.workDir)

    def PrepareChangelog(self):
        if os.path.exists(self.pkgSharedDir):
            shutil.rmtree(self.pkgSharedDir)
        os.makedirs(self.pkgSharedDir)
        os.chdir(self.pkgSharedDir)

        with open('changelog', 'w') as changelog:
            changelog.write(self.descript + '\n')
            changelog.write('For more detailed information, pelase check ' + self.targetRepo + 'AMDVLK/releases/tag/' + self.buildTag)

        # amd64 and i386 packages must share the same changelog.Debian.gz with the same timestamp, else there will be conflict
        # when installing both packages on the same system
        shutil.copy('changelog', 'changelog.Debian')
        os.system('gzip -9 changelog.Debian')
        shutil.copy('changelog.Debian.gz', self.driverRoot + 'xgl/changelog.Debian.gz')

    def ArchiveAmdllpcTools(self, arch):
        toolsDir = 'amdllpc_' + arch

        os.chdir(self.workDir)
        if os.path.exists(toolsDir):
            shutil.rmtree(toolsDir)
        os.makedirs(toolsDir)

        os.system('cp ' + os.path.join(self.driverRoot, self.buildDir + '/compiler/llpc/amdllpc') + ' ' + toolsDir)
        os.system('zip -r ' + toolsDir + '.zip ' + toolsDir)

    def start(self):
        self.GetOpt()
        self.SyncAMDVLK()
        self.CheckoutDriver()
        self.Build()

if __name__ == '__main__':
    worker = Worker()
    worker.start()

