#!/usr/bin/python3

# This script is used to generate release packages from release tag

# Before running this script, please install dependency packages with
# pip3 install gitpython
# pip3 install PyGithub

import sys
import os
import git
import shutil
from optparse import OptionParser
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
        self.accessToken  = ''

    def IsBuildTagNewer(self):
        #TODO: use a more flexible algorithm
        return self.buildTag > self.diffTag

    def SetupBuildTagInfo(self, inputTag):
        os.chdir(self.srcDir)
        repoName = 'AMDVLK'
        if os.path.exists(repoName):
            shutil.rmtree(repoName)
        cloneCmd = 'git clone ' + self.targetRepo + repoName
        if os.system(cloneCmd):
            eprint(cloneCmd + ' failed')
            exit(-1)
        repo = git.Repo(repoName)
        if not repo.tags:
            eprint("No tags found from AMDVLK")
            exit(-1)

        if not inputTag:
            self.buildTag = repo.tags[-1].name
        elif inputTag in repo.tags:
            self.buildTag = inputTag
        else:
            eprint("You input an invalid tag: " + inputTag)
            exit(-1)

        self.version = self.buildTag[2:]
        tagRef = repo.tag('refs/tags/' + self.buildTag)
        if self.IsBuildTagNewer():
            self.descript = tagRef.object.message
        else:
            self.descript = tagRef.commit.message

    def GetOpt(self):
        parser = OptionParser()

        parser.add_option("-w", "--workDir", action="store",
                          type="string",
                          dest="workDir",
                          help="Specify the directory to build, default is current working directory")

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
        else:
            print("The source code is not specified, downloading from github to: " + self.workDir)

        self.srcDir = self.workDir + "/amdvlk_src/"
        self.pkgDir = self.workDir + "/amdvlk_pkg/"
        self.pkgSharedDir = self.workDir + "/pkgShared/"
        self.driverRoot = os.path.join(self.srcDir, 'drivers/')
        if not os.path.exists(self.srcDir):
            os.makedirs(self.srcDir)

        if options.targetRepo:
            self.targetRepo = options.targetRepo.rstrip('/') + '/'
        print("The target repo is " + self.targetRepo)

        self.SetupBuildTagInfo(options.buildTag)
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

    def SyncAMDVLK(self):
        # Sync all amdvlk repoes and checkout AMDVLK to specified tag
        os.chdir(self.srcDir)

        repoName = 'AMDVLK'
        initCmd='repo init --depth=1 -u ' + self.targetRepo + repoName + ' -b refs/tags/' + self.buildTag
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

