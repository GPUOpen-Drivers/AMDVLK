#!/usr/bin/python3

# This script is used to generate release packages from release tag

# Before running this script, please install dependency packages with
# pip3 install gitpython
# pip3 install PyGithub

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
from github import Github

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

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
        self.diffTag      = 'v-2022.Q3.1'
        self.validTags    = []

    def UpdateValidTags(self, repoPath):
        repo = Github().get_repo(repoPath)
        allTags = repo.get_tags()
        validTags = []
        for t in allTags:
            if t.name[:2] == 'v-': validTags.append(t.name)
        if not validTags:
            eprint("No valid tags found from AMDVLK")
            exit(-1)
        self.validTags = sorted(validTags, reverse=True)

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
                          help="Specify the tag to build, e.g.: \"v-2022.Q3.1\", default is latest")

        (options, args) = parser.parse_args()

        if options.workDir:
            print("The source code is under %s" % (options.workDir))
            self.workDir = os.path.abspath(options.workDir)
            self.srcDir  = self.workDir + "/amdvlk_src/"
            self.pkgDir  = self.workDir + "/amdvlk_pkg/"
        else:
            print("The source code is not specified, downloading from github to: " + self.workDir)

        if not os.path.exists(self.srcDir):
            os.makedirs(self.srcDir)

        if options.targetRepo:
            self.targetRepo = options.targetRepo

        print("The target repo is " + self.targetRepo)

        self.UpdateValidTags(self.targetRepo.strip('/ ').split('/')[-1] + '/AMDVLK')
        if options.buildTag:
            if options.buildTag in self.validTags:
                self.buildTag = options.buildTag
            else:
                eprint("You input an invalid tag: " + options.buildTag)
                exit(-1)
        else:
            self.buildTag = self.validTags[0]
        print("The build tag is " + self.buildTag)

    def DistributionType(self):
        result = os.popen('lsb_release -is').read().strip()
        if (result == 'Ubuntu'):
            return result
        elif (result == 'RedHatEnterprise' or result == 'RedHatEnterpriseWorkstation'):
            return 'RHEL'
        else:
            eprint('Unknown Linux distribution: ' + result)
            sys.exit(-1)

    def IsBuildTagNewer(self):
        #TODO: use a more flexible algorithm
        return self.buildTag > self.diffTag

    def SyncAMDVLK(self):
        # Sync all amdvlk repoes and checkout AMDVLK to specified tag
        os.chdir(self.srcDir)

        repoName = 'AMDVLK'
        initCmd='repo init -u ' + self.targetRepo + repoName + ' -b refs/tags/' + self.buildTag
        syncCmd='repo sync -j8'
        if self.IsBuildTagNewer():
            initCmd += ' -m build_with_tools.xml'
        else:
            initCmd += ' -m default.xml'
        if os.system(initCmd):
            eprint(initCmd + ' failed')
            exit(-1)
        if os.system(syncCmd):
            eprint('repo sync failed')
            exit(-1)

        # Simply use repo command instead of reading from manifest to get path
        amdvlkPath = os.popen('repo list --path-only ' + repoName).read().strip()
        self.driverRoot = self.srcDir + amdvlkPath[:-len(repoName)]
        repo = git.Repo(amdvlkPath)
        for tagRef in repo.tags:
            if self.buildTag == tagRef.name:
                if self.IsBuildTagNewer():
                    self.descript = tagRef.tag.message
                else:
                    self.descript = tagRef.commit.message
                self.version = self.buildTag[2:]
                break

        repo.git.checkout(self.buildTag)

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
                eprint('SPVGEN: fetch external sources failed')
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
            eprint(cmakeName + cmakeFlags + cFlags + ' failed')
            exit(-1)

        if os.system('cmake --build ' + self.buildDir):
            eprint('build amdvlk failed')
            exit(-1);

        # Make driver package
        if os.system('cmake --build ' + self.buildDir + ' --target makePackage'):
            eprint('make driver package failed')

        # Build spvgen
        if os.system('cmake --build ' + self.buildDir + ' --target spvgen'):
            eprint('SPVGEN: build failed')
            exit(-1);

        # Build amdllpc
        if os.system('cmake --build ' + self.buildDir + ' --target amdllpc'):
            eprint('build amdllpc failed')
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
        self.Build()

if __name__ == '__main__':
    worker = Worker()
    worker.start()

