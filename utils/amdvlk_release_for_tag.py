#!/usr/bin/python3

# This script is used to build the AMD open source vulkan driver and make a deb package from github for tags.

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
from github import Github

DriverVersionStub = 'DriverVersionStub'
ArchitectureStub  = 'ArchitectureStub'
Control = "Package: amdvlk\n\
Version: " + DriverVersionStub + "\n\
Architecture: " + ArchitectureStub + "\n\
Maintainer: Advanced Micro Devices (AMD) <gpudriverdevsupport@amd.com>\n\
Depends: libc6 (>= 2.17), libgcc1 (>= 1:3.4), libstdc++6 (>= 5.2)\n\
Conflicts: amdvlk\n\
Replaces: amdvlk\n\
Section: libs\n\
Priority: optional\n\
Multi-Arch: same\n\
Homepage: https://github.com/GPUOpen-Drivers/AMDVLK\n\
Description: AMD Open Source Driver for Vulkan"

SPEC = "Name: amdvlk\n\
Version: " + DriverVersionStub + "\n\
Release: el\n\
Summary: AMD Open Source Driver for Vulkan\n\
URL: https://github.com/GPUOpen-Drivers/AMDVLK\n\
License: MIT\n\
Group: System Environment/Libraries\n\
Vendor: Advanced Micro Devices (AMD) <gpudriverdevsupport@amd.com>\n\
Buildarch: x86_64\n\n\
%description\n\
%prep\n\
%build\n\
%pre\n\
%post\n\
%preun\n\
%postun\n\
%files\n\
/usr/lib64/amdvlk64.so\n\
/etc/vulkan/icd.d/amd_icd64.json\n\
/etc/vulkan/implicit_layer.d/amd_icd64.json\n\
/usr/share/doc/amdvlk/copyright\n\
/usr/share/doc/amdvlk/changelog\n\
%changelog"

ChangeHeader = "vulkan-amdgpu (" + DriverVersionStub + ") unstable; urgency=low\n\
\n\
  * Checkout from github:"

CopyRight = "The MIT License (MIT)\n\
\n\
Copyright (c) 2018 Advanced Micro Devices, Inc.\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all\n\
copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\
SOFTWARE."

class Worker:
    def __init__(self):
        self.workDir      = os.getcwd()
        self.srcDir       = self.workDir + "/amdvlk_src/"
        self.pkgDir       = self.workDir + "/amdvlk_pkg/"
        self.pkgSharedDir = os.path.join(self.workDir, 'pkgShared')
        self.branch       = 'master'
        self.components   = ['xgl', 'pal', 'llpc', 'spvgen', 'llvm-project', 'MetroHash', 'CWPack']
        self.tagList      = []
        self.relTagList   = [] # The tags already released on github
        self.commits      = {'xgl':'', 'pal':'', 'llpc':'', 'spvgen':'', 'llvm-project':'', 'MetroHash':'', 'CWPack':''}
        self.descript     = ""
        self.targetRepo   = 'https://github.com/GPUOpen-Drivers/'
        self.choice       = 'build'
        self.distro       = self.DistributionType()

    def GetOpt(self):
        parser = OptionParser()

        parser.add_option("-w", "--workDir", action="store",
                          type="string",
                          dest="workDir",
                          help="Specify the location of source code, or download it from github")

        parser.add_option("-a", "--accessToken", action="store",
                          type="string",
                          dest="accessToken",
                          help="Specify the accessToken to access github")

        parser.add_option("-t", "--targetRepo", action="store",
                          type="string",
                          dest="targetRepo",
                          help="Specify the target repo of github, default is " + self.targetRepo)

        parser.add_option("-c", "--choice", action="store",
                          type="string",
                          dest="choice",
                          help="Build package or release it? Default is: " + self.choice)

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

        if options.accessToken:
            self.accessToken = options.accessToken
        else:
            print("Please specify the access token to github, exiting...")
            sys.exit(-1)

        if options.targetRepo:
            self.targetRepo = options.targetRepo

        print("The target repos is " + self.targetRepo)

        if options.choice:
            self.choice = options.choice
        else:
            print('Please specify choice, build or release?')
            sys.exit(-1)

    def ConnectGithub(self):
        foundRepo = False
        self.github = Github(self.accessToken)
        for repo in self.github.get_user().get_repos():
            if (repo.name == 'AMDVLK'):
                self.repo = repo
                foundRepo = True

        if (foundRepo == False):
            print("Fatal: AMDVLK repo is not found")
            sys.exit(-1)

    def DistributionType(self):
        result = os.popen('lsb_release -is').read().strip()
        if (result == 'Ubuntu'):
            return result
        elif (result == 'RedHatEnterprise' or result == 'RedHatEnterpriseWorkstation'):
            return 'RHEL'
        else:
            print('Unknown Linux distribution: ' + result)
            sys.exit(-1)

    def GetReleasedTagsOnGithub(self):
        releases = self.repo.get_releases()

        for release in releases:
            print(release.tag_name + " is released already")
            self.relTagList.append(release.tag_name)

    def CloneAMDVLK(self):
        # Clone the AMDVLK and get the released tag list
        # The released tag name must be with format "v-major.minor", for example, "v-1.0"
        os.chdir(self.srcDir)
        # Remove it if it exists
        if os.path.exists(self.srcDir + 'AMDVLK'):
            shutil.rmtree(self.srcDir + 'AMDVLK')

        git.Git().clone(self.targetRepo + 'AMDVLK')
        repo = git.Repo('AMDVLK')
        tags = repo.git.tag()
        release_tag_pattern = r'(^v-)'
        for tag in tags.split('\n'):
            if re.findall(release_tag_pattern, tag):
                self.tagList.append(tag)
                print(tag + ' is added to tag lists')

    def DownloadAMDVLKComponents(self):
        os.chdir(self.srcDir)

        for i in self.components:
            if not os.path.exists(self.srcDir + i):
                print("Downloading " + i + ".....")
                git.Git().clone(self.targetRepo + i)

            repo = git.Repo(i)
            repo.git.clean('-xdf')
            # Clean the submodule
            repo.git.clean('-xdff')
            if (i == 'llvm-project'):
                repo.git.checkout('remotes/origin/amd-gfx-gpuopen-' + self.branch, B='amd-gfx-gpuopen-' + self.branch)
            elif (i == 'MetroHash' or i == 'CWPack'):
                repo.git.checkout('remotes/origin/amd-master', B='amd-master')
            else:
                repo.git.checkout('remotes/origin/' + self.branch, B=self.branch)
            repo.git.pull()

    def GetRevisions(self, tag):
        os.chdir(self.srcDir)
        repo = git.Repo('AMDVLK')
        repo.git.checkout(tag)
        self.descript = repo.head.commit.message

        # Update version
        self.version = tag[2:]
        print("Building for " + tag + " with version " + self.version)

        # Get the commits from default.xml
        srcFileName = 'AMDVLK/default.xml'
        srcFile     = open(srcFileName, 'r')
        lines       = srcFile.readlines()

        for line in lines:
            for i in self.commits:
                index = line.find("revision=");
                if (index > -1) and (line.find(i) > -1):
                    startIndex      = index + len("revision=\"")
                    stopIndex       = line.find("\"", startIndex)
                    self.commits[i] = line[startIndex:stopIndex]
                    print(i + ":" + self.commits[i])
                    # Checkout the commits
                    repo = git.Repo(i)
                    repo.git.clean('-xdf')
                    # Clean the submodule
                    repo.git.clean('-xdff')
                    repo.git.checkout(self.commits[i])
                    break

        srcFile.close()
    def Build(self):
        if self.distro == 'Ubuntu':
            self.MakeDriver('64')
            self.MakeDriver('32')
        elif self.distro == 'RHEL':
            self.MakeDriver('64')

    def MakeDriver(self, arch):
        cmakeName = 'cmake '
        if (self.distro == 'RHEL'):
            cmakeName = 'source scl_source enable devtoolset-7 && cmake3 '

        # fetch spvgen resources
        os.chdir(self.srcDir + 'spvgen/external')
        if os.system('python fetch_external_sources.py'):
            print("SPVGEN: fetch external sources failed")
            exit(-1)

        # build amdvlk
        buildDir   = 'rbuild64' if arch == '64' else 'rbuild32'
        cmakeFlags = ' -H. -G Ninja -B' + buildDir + ' -DCMAKE_BUILD_TYPE=Release -DBUILD_WAYLAND_SUPPORT=ON'
        cFlags     = '' if arch == '64' else ' -DCMAKE_C_FLAGS=\"-m32 -march=i686\" -DCMAKE_CXX_FLAGS=\"-m32 -march=i686\"'

        os.chdir(self.srcDir + 'xgl/')
        if os.path.exists(buildDir):
            shutil.rmtree(buildDir)
        os.makedirs(buildDir)

        if os.system(cmakeName + cmakeFlags + cFlags):
            print(cmakeName + cmakeFlags + cFlags + ' failed')
            exit(-1)

        os.chdir(buildDir);
        if os.system('ninja'):
            print("build amdvlk failed")
            exit(-1);

        # build spvgen
        if os.system('ninja spvgen'):
            print('SPVGEN: build failed')
            exit(-1);

        # build amdllpc
        if os.system('ninja amdllpc'):
            print('build amdllpc failed')
            exit(-1);

    def PreparePkgSharedResources(self):
        if os.path.exists(self.pkgSharedDir):
            shutil.rmtree(self.pkgSharedDir)
        os.makedirs(self.pkgSharedDir)
        os.chdir(self.pkgSharedDir)

        change_file     = open('changelog', 'w')
        pkgChangeHeader = ChangeHeader.replace(DriverVersionStub, self.version)
        change_file.write(pkgChangeHeader + '\n')

        for i in self.components:
            change_file.write("    " + self.targetRepo + i + ": " + self.branch + "--" + self.commits[i] + '\n')
        change_file.close()

        os.system('cp changelog changelog.Debian')
        os.system('gzip -9 -c ' + 'changelog.Debian' + ' >| ' + 'changelog.Debian.gz')
        os.remove('changelog.Debian')

        copyright_file = open('copyright', 'w')
        copyright_file.write(CopyRight + '\n')
        copyright_file.close()

    def MakeDebPackage(self, arch):
        if not os.path.exists(self.pkgDir):
            os.makedirs(self.pkgDir)
        os.chdir(self.pkgDir)

        if os.path.exists(arch):
            shutil.rmtree(arch)
        os.makedirs(arch)
        os.chdir(arch)

        icdInstallDir    = 'usr/lib/x86_64-linux-gnu' if arch == 'amd64' else 'usr/lib/i386-linux-gnu'
        jsonInstallDir   = 'etc/vulkan/icd.d'
        implicitLayerDir = 'etc/vulkan/implicit_layer.d'
        docInstallDir    = 'usr/share/doc/amdvlk'
        icdName          = 'amdvlk64.so' if arch == 'amd64' else 'amdvlk32.so'
        icdBuildDir      = 'xgl/rbuild64/icd' if arch == 'amd64' else 'xgl/rbuild32/icd'
        jsonName         = 'amd_icd64.json' if arch == 'amd64' else 'amd_icd32.json'

        os.makedirs(icdInstallDir)
        os.makedirs(docInstallDir)
        os.makedirs(jsonInstallDir)
        os.makedirs(implicitLayerDir)
        os.makedirs('DEBIAN')

        os.system('cp ' + os.path.join(self.srcDir, icdBuildDir, icdName) + ' ' + icdInstallDir)
        os.system('strip ' + os.path.join(icdInstallDir, icdName))
        os.system('cp ' + os.path.join(self.srcDir, 'AMDVLK/json/Ubuntu', jsonName) + ' ' + jsonInstallDir)
        #os.system('cp ' + os.path.join(self.srcDir, 'AMDVLK/json/Ubuntu', jsonName) + ' ' + implicitLayerDir)

        debControl = Control.replace(DriverVersionStub, self.version).replace(ArchitectureStub, arch)
        control_file = open("DEBIAN/control",'w')
        control_file.write(debControl + '\n')
        control_file.close()

        os.system('cp ' + os.path.join(self.pkgSharedDir, 'changelog.Debian.gz') + ' ' + os.path.join(docInstallDir, 'changelog.Debian.gz'))
        os.system('cp ' + os.path.join(self.pkgSharedDir, 'copyright') + ' ' + docInstallDir)

        pkg_content = os.path.join(icdInstallDir, icdName) + ' ' + os.path.join(jsonInstallDir, jsonName) + ' ' + os.path.join(implicitLayerDir, jsonName) + ' ' \
                      + os.path.join(docInstallDir,'changelog.Debian.gz') + ' ' + os.path.join(docInstallDir, 'copyright') + ' '
        os.system('md5sum ' + pkg_content + '> DEBIAN/md5sums')

        os.chdir(self.workDir)
        os.system('dpkg -b ' + os.path.join(self.pkgDir, arch) + ' amdvlk_' + self.version + '_' + arch + '.deb')

    def ArchiveAmdllpcTools(self, arch):
        toolsDir = 'amdllpc_' + arch

        os.chdir(self.workDir)
        if os.path.exists(toolsDir):
            shutil.rmtree(toolsDir)
        os.makedirs(toolsDir)

        spvgenName      = 'spvgen.so'
        spvgenBuildDir  = 'xgl/rbuild64/spvgen' if arch == 'amd64' else 'xgl/rbuild32/spvgen'
        amdllpcName     = 'amdllpc'
        amdllpcBuildDir = 'xgl/rbuild64/compiler/llpc' if arch == 'amd64' else 'xgl/rbuild32/compiler/llpc'

        os.system('cp ' + os.path.join(self.srcDir, amdllpcBuildDir, amdllpcName) + ' ' + toolsDir)
        os.system('cp ' + os.path.join(self.srcDir, spvgenBuildDir, spvgenName) + ' ' + toolsDir)
        os.system('zip -r ' + toolsDir + '.zip ' + toolsDir)

    def MakeRpmPackage(self):
        rpmbuild_dir = os.path.join(os.getenv('HOME'), 'rpmbuild')
        rpmbuildroot_dir = 'BUILDROOT'
        rpmspec_dir = 'SPEC'
        rpmspec_file_name = 'amdvlk.spec'
        icd_install_dir = 'usr/lib64'
        doc_install_dir = 'usr/share/doc/amdvlk'
        json_install_dir = 'etc/vulkan/icd.d'
        implicit_layer_dir = 'etc/vulkan/implicit_layer.d'
        icd_name = 'amdvlk64.so'
        json_name = 'amd_icd64.json'

        if os.path.exists(rpmbuild_dir):
            shutil.rmtree(rpmbuild_dir)
        os.makedirs(rpmbuild_dir)
        os.chdir(rpmbuild_dir)
        os.makedirs(rpmbuildroot_dir)
        os.makedirs(rpmspec_dir)

        rpm_spec = SPEC.replace(DriverVersionStub, self.version)
        spec_file = open(os.path.join(rpmspec_dir, rpmspec_file_name), 'w')
        spec_file.write(rpm_spec + '\n')
        spec_file.close()

        os.chdir(rpmbuildroot_dir)
        packagename = 'amdvlk-' + self.version + '-el.x86_64'
        os.makedirs(packagename)
        os.chdir(packagename)
        os.makedirs(icd_install_dir)
        os.makedirs(doc_install_dir)
        os.makedirs(json_install_dir)
        os.makedirs(implicit_layer_dir)

        os.system('cp ' + os.path.join(self.srcDir, 'xgl/rbuild64/icd', icd_name) + ' ' + icd_install_dir)
        os.system('strip ' + os.path.join(icd_install_dir, icd_name))
        os.system('cp ' + os.path.join(self.srcDir, 'AMDVLK/json/Redhat', json_name) + ' ' + json_install_dir)
        #os.system('cp ' + os.path.join(self.srcDir, 'AMDVLK/json/Redhat', json_name) + ' ' + implicit_layer_dir)

        os.system('cp ' + os.path.join(self.pkgSharedDir, 'changelog') + ' ' + doc_install_dir)
        os.system('cp ' + os.path.join(self.pkgSharedDir, 'copyright') + ' ' + doc_install_dir)

        os.chdir(rpmbuild_dir)
        os.chdir(rpmspec_dir)
        os.system('rpmbuild -bb ' + rpmspec_file_name)
        os.chdir(rpmbuild_dir)
        os.system('cp RPMS/x86_64/' + packagename + '.rpm ' + self.workDir)

    def Package(self):
        self.PreparePkgSharedResources()

        if (self.distro == 'Ubuntu'):
            self.MakeDebPackage('amd64')
            self.ArchiveAmdllpcTools('amd64')
            self.MakeDebPackage('i386')
            self.ArchiveAmdllpcTools('i386')
        elif (self.distro == 'RHEL'):
            self.MakeRpmPackage()

    def Release(self, tag):
        os.chdir(self.workDir)

        rpmPackageName = 'amdvlk-' + self.version + '-el.x86_64.rpm'
        debPackage64bitName = 'amdvlk_' + self.version + '_amd64.deb'
        debPackage32bitName = 'amdvlk_' + self.version + '_i386.deb'
        amdllpc64bitName = 'amdllpc_amd64.zip'
        amdllpc32bitName = 'amdllpc_i386.zip'

        if not os.path.isfile(rpmPackageName):
            print('Can not find package: ' + rpmPackageName)
            sys.exit(-1)
        if not os.path.isfile(debPackage64bitName):
            print('Can not find package: ' + debPackage64bitName)
            sys.exit(-1)
        if not os.path.isfile(debPackage32bitName):
            print('Can not find package: ' + debPackage32bitName)
            sys.exit(-1)
        if not os.path.isfile(amdllpc64bitName):
            print('Can not find package: ' + amdllpc64bitName)
            sys.exit(-1)
        if not os.path.isfile(amdllpc32bitName):
            print('Can not find package: ' + amdllpc32bitName)
            sys.exit(-1)


        releaseNote = '[Driver installation instruction](https://github.com/GPUOpen-Drivers/AMDVLK#install-with-pre-built-driver) \n\n'
        formated_str = self.descript.replace("New feature and improvement", "## New feature and improvement")
        formated_str = formated_str.replace("Issue fix", "## Issue fix")
        releaseNote += formated_str

        newRelease = self.repo.create_git_release(tag, tag, releaseNote, False, False)

        newRelease.upload_asset(rpmPackageName, rpmPackageName + '(RedHat 7.8 8.2)')
        newRelease.upload_asset(debPackage64bitName, debPackage64bitName + '(Ubuntu 18.04 20.04)')
        newRelease.upload_asset(debPackage32bitName, debPackage32bitName + '(Ubuntu 18.04 20.04)')
        newRelease.upload_asset(amdllpc64bitName, amdllpc64bitName)
        newRelease.upload_asset(amdllpc32bitName, amdllpc32bitName)

    def start(self):
        self.GetOpt()
        self.ConnectGithub()
        self.GetReleasedTagsOnGithub()
        self.CloneAMDVLK()
        # Build and package if there is any tag un-released.
        downloaded   = False
        ReleaseCount = 0
        for tag in self.tagList:
            if tag not in self.relTagList:
                ReleaseCount += 1
                if not downloaded:
                    self.DownloadAMDVLKComponents()
                    downloaded = True
                self.GetRevisions(tag)
                if (self.choice == 'build'):
                    self.Build()
                    self.Package()
                    print("The package is generated successfully for " + tag)
                elif (self.choice == 'release'):
                    self.Release(tag)
                    print("Released " + tag + " successfully")

        if ReleaseCount == 0:
            print("All of the tags are released!")

if __name__ == '__main__':
    worker = Worker()
    worker.start()

