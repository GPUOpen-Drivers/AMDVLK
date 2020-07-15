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

DriverVersion = "Thisisjustastub"
Control = "Package: amdvlk\n\
Version: " + DriverVersion + "\n\
Architecture: amd64\n\
Maintainer: Advanced Micro Devices (AMD) <gpudriverdevsupport@amd.com>\n\
Depends: libc6 (>= 2.17), libgcc1 (>= 1:3.4), libstdc++6 (>= 5.2)\n\
Conflicts: amdvlk\n\
Replaces: amdvlk\n\
Section: libs\n\
Priority: optional\n\
Multi-Arch: same\n\
Homepage: https://github.com/GPUOpen-Drivers/AMDVLK\n\
Description: AMD Open Source Driver for Vulkan";

SPEC = "Name: amdvlk\n\
Version: " + DriverVersion + "\n\
Release: el\n\
Summary: AMD Open Source Driver for Vulkan\n\
License: MIT\n\
Group: AMD\n\
Vendor: AMD\n\
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
/usr/lib64/spvgen.so\n\
/etc/vulkan/icd.d/amd_icd64.json\n\
/usr/share/doc/amdvlk/copyright\n\
%changelog"

ChangeHeader = "vulkan-amdgpu (" + DriverVersion + ") unstable; urgency=low\n\
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
        self.workDir    = os.getcwd();
        self.srcDir     = self.workDir + "/amdvlk_src/";
        self.pkgDir     = self.workDir + "/amdvlk_pkg/";
        self.branch     = 'master';
        self.components = ['xgl', 'pal', 'llpc', 'spvgen', 'llvm-project', 'MetroHash', 'CWPack'];
        self.tagList    = [];
        self.relTagList = []; # The tags already released on github
        self.commits    = {'xgl':'', 'pal':'', 'llpc':'', 'spvgen':'', 'llvm-project':'', 'MetroHash':'', 'CWPack':''};
        self.descript   = "";
        self.basever    = "1.1.";
        self.targetRepo = 'https://github.com/GPUOpen-Drivers/';
        self.type       = 'build';
        self.distro     = self.DistributionType()

    def GetOpt(self):
        parser = OptionParser();

        parser.add_option("-w", "--workDir", action="store",
                          type="string",
                          dest="workDir",
                          help="Specify the location of source code, or download it from github")

        parser.add_option("-a", "--accessToken", action="store",
                          type="string",
                          dest="accessToken",
                          help="Specify the accessToken to access github")

        parser.add_option("-t", "--type", action="store",
                          type="string",
                          dest="type",
                          help="Build package or release it? Default is: " + self.type)

        (options, args) = parser.parse_args()

        if options.workDir:
            print("The source code is under %s" % (options.workDir));
            self.workDir = options.workDir;
            self.srcDir  = self.workDir + "/amdvlk_src/";
            self.pkgDir  = self.workDir + "/amdvlk_pkg/";
        else:
            print("The source code is not specified, downloading from github to: " + self.workDir);

        if not os.path.exists(self.srcDir):
            os.makedirs(self.srcDir);

        if options.accessToken:
            self.accessToken = options.accessToken;
        else:
            print("Please specify the access token to github, exiting...");
            sys.exit(-1);

        if options.type:
            self.type = options.type;
        else:
            print('Please specify type, build or release?')
            sys.exit(-1)

        print('The type of the action is: ' + self.type)
        print("The target repo is " + self.targetRepo);

    def ConnectGithub(self):
        foundRepo = False;
        self.github = Github(login_or_token = self.accessToken,retry=10);
        for repo in self.github.get_user().get_repos():
            if (repo.name == 'AMDVLK'):
                self.repo = repo;
                foundRepo = True;

        if (foundRepo == False):
            print("Fatal: AMDVLK repo is not found");
            sys.exit(-1);

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
        releases = self.repo.get_releases();

        for release in releases:
            print(release.tag_name + " is released already");
            self.relTagList.append(release.tag_name);

    def CloneAMDVLK(self):
        # Clone the AMDVLK and get the released tag list
        # The released tag name must be with format "v-major.minor", for example, "v-1.0"
        os.chdir(self.srcDir);
        # Remove it if it exists
        if os.path.exists(self.srcDir + 'AMDVLK'):
            shutil.rmtree(self.srcDir + 'AMDVLK');

        git.Git().clone(self.targetRepo + 'AMDVLK');
        repo = git.Repo('AMDVLK');
        tags = repo.git.tag();
        release_tag_pattern = r'(^v-)'
        for tag in tags.split('\n'):
            if re.findall(release_tag_pattern, tag):
                self.tagList.append(tag);
                print(tag + ' is added to tag lists');

        '''
        #Get the base version from AMDVLK/json/Ubuntu
        json_file = open("AMDVLK/json/Ubuntu/amd_icd64.json",'r');
        lines     = json_file.readlines();

        for line in lines:
            location = line.find("\"api_version\":");
            if (location > -1):
                location = location + len("\"api_version\":") + 2;
                self.basever = line[location:-2]
                break;
        '''

    def DownloadAMDVLKComponents(self):
        os.chdir(self.srcDir);

        for i in self.components:
            if not os.path.exists(self.srcDir + i):
                print("Downloading " + i + ".....");
                git.Git().clone(self.targetRepo + i);

            repo = git.Repo(i);
            repo.git.clean('-xdf');
            # Clean the submodule
            repo.git.clean('-xdff');
            if (i == 'llvm-project'):
                repo.git.checkout('remotes/origin/amd-gfx-gpuopen-' + self.branch, B='amd-gfx-gpuopen-' + self.branch);
            elif (i == 'MetroHash' or i == 'CWPack'):
                repo.git.checkout('remotes/origin/amd-master', B='amd-master');
            else:
                repo.git.checkout('remotes/origin/' + self.branch, B=self.branch);
            repo.git.pull();

    def GetRevisions(self, tag):
        os.chdir(self.srcDir);
        repo = git.Repo('AMDVLK');
        repo.git.checkout(tag);
        self.descript = repo.head.commit.message;

        # Update version
        # self.version = self.basever + "-" + tag;
        self.version = tag[2:];
        print("Building for " + tag + " with version " + self.version);

        # Get the commits from default.xml
        srcFileName = 'AMDVLK/default.xml';
        srcFile     = open(srcFileName, 'r');
        lines       = srcFile.readlines();

        for line in lines:
            for i in self.commits:
                index = line.find("revision=");
                if (index > -1) and (line.find(i) > -1):
                    startIndex      = index + len("revision=\"");
                    stopIndex       = line.find("\"", startIndex);
                    self.commits[i] = line[startIndex:stopIndex];
                    print(i + ":" + self.commits[i]);
                    # Checkout the commits
                    repo = git.Repo(i);
                    repo.git.clean('-xdf');
                    # Clean the submodule
                    repo.git.clean('-xdff');
                    repo.git.checkout(self.commits[i]);
                    break;

        srcFile.close();

    def Build(self):
        cmakeName = 'cmake'
        if (self.distro == 'RHEL'):
            cmakeName = 'source scl_source enable devtoolset-7 && cmake3'
        # build amdvlk64.so
        os.chdir(self.srcDir + 'xgl/');
        if os.system(cmakeName + ' -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DBUILD_WAYLAND_SUPPORT=ON'):
            print(cmakeName + " -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release failed");
            exit(-1);

        os.chdir('rbuild64');
        if os.system('make -j8'):
            print("build amdvlk failed");
            exit(-1);

        # build spvgen
        os.chdir(self.srcDir + 'spvgen/external');
        if os.system('python fetch_external_sources.py'):
            print("SPVGEN: fetch external sources failed");
            exit(-1);

        os.chdir(self.srcDir + 'spvgen/');
        if os.system(cmakeName + ' -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=\"-Wno-error=unused-variable\"'):
            print("SPVGEN: " + cmakeName + " -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=\"-Wno-error=unused-variable\" failed");
            exit(-1);

        os.chdir('rbuild64');
        if os.system('make -j8'):
            print("build spvgen failed");
            exit(-1);


    def MakeDebPackage(self):
        global Control;
        global ChangeHeader;
        global CopyRight;
        global DriverVersion;

        os.chdir(self.workDir);

        if os.path.exists(self.pkgDir):
            os.system('rm -rf ' + self.pkgDir)

        os.makedirs(self.pkgDir);
        os.chdir(self.pkgDir);
 
        os.makedirs('usr/lib/x86_64-linux-gnu');
        os.makedirs('usr/share/doc/amdvlk');
        os.makedirs('etc/vulkan/icd.d');
        os.makedirs('DEBIAN');

        os.system('cp ' + self.srcDir + '/xgl/rbuild64/icd/amdvlk64.so ' + 'usr/lib/x86_64-linux-gnu/');
        os.system('strip usr/lib/x86_64-linux-gnu/amdvlk64.so');
        os.system('cp ' + self.srcDir + '/spvgen/rbuild64/spvgen.so ' + 'usr/lib/x86_64-linux-gnu/');
        os.system('strip usr/lib/x86_64-linux-gnu/spvgen.so');
        os.system('cp ' + self.srcDir + '/AMDVLK/json/Ubuntu/amd_icd64.json ' + 'etc/vulkan/icd.d/amd_icd64.json');

        Control      = Control.replace(DriverVersion, self.version);
        ChangeHeader = ChangeHeader.replace(DriverVersion, self.version);

        control_file = open("DEBIAN/control",'w');
        control_file.write(Control + '\n');
        control_file.close();

        change_file = open("usr/share/doc/amdvlk/changelog.Debian",'w');
        change_file.write(ChangeHeader + '\n');

        for i in self.components:
            change_file.write("    " + self.targetRepo + i + ": " + self.branch + "--" + self.commits[i] + '\n');
        change_file.close()

        os.system('gzip -9 -c usr/share/doc/amdvlk/changelog.Debian >| usr/share/doc/amdvlk/changelog.Debian.gz');
        os.system('rm -rf usr/share/doc/amdvlk/changelog.Debian');

        copyright_file = open("usr/share/doc/amdvlk/copyright",'w');
        copyright_file.write(CopyRight + '\n');
        copyright_file.close()

        os.system('md5sum usr/lib/x86_64-linux-gnu/amdvlk64.so usr/lib/x86_64-linux-gnu/spvgen.so etc/vulkan/icd.d/amd_icd64.json usr/share/doc/amdvlk/changelog.Debian.gz usr/share/doc/amdvlk/copyright > DEBIAN/md5sums')

        os.chdir(self.workDir);
        os.system('dpkg -b amdvlk_pkg amdvlk_' + self.version + '_amd64.deb');

    def MakeRpmPackage(self):
        global CopyRight;
        global DriverVersion;
        global SPEC;

        rpmbuild_dir = os.getenv('HOME') + '/rpmbuild'
        if os.path.exists(rpmbuild_dir):
            os.system('rm -rf ' + rpmbuild_dir)
        os.makedirs(rpmbuild_dir)
        os.chdir(rpmbuild_dir)
        os.makedirs('BUILDROOT')
        os.makedirs('SPEC')

        SPEC = SPEC.replace(DriverVersion, self.version)
        control_file = open('SPEC/amdvlk.spec', 'w')
        control_file.write(SPEC + '\n')
        control_file.close()

        os.chdir('BUILDROOT')
        packagename = 'amdvlk-' + self.version + '-el.x86_64'
        os.makedirs(packagename)
        os.chdir(packagename)
        os.makedirs('usr/lib64');
        os.makedirs('usr/share/doc/amdvlk');
        os.makedirs('etc/vulkan/icd.d');

        os.system('cp ' + self.srcDir + '/xgl/rbuild64/icd/amdvlk64.so ' + 'usr/lib64');
        os.system('strip usr/lib64/amdvlk64.so');
        os.system('cp ' + self.srcDir + '/spvgen/rbuild64/spvgen.so ' + 'usr/lib64');
        os.system('strip usr/lib64/spvgen.so');
        os.system('cp ' + self.srcDir + '/AMDVLK/json/Redhat/amd_icd64.json ' + 'etc/vulkan/icd.d/amd_icd64.json');

        copyright_file = open("usr/share/doc/amdvlk/copyright",'w');
        copyright_file.write(CopyRight + '\n');
        copyright_file.close()

        os.chdir(rpmbuild_dir)
        os.chdir('SPEC')
        os.system('rpmbuild -bb ./amdvlk.spec')
        os.chdir(rpmbuild_dir)
        os.system('cp RPMS/x86_64/' + packagename + '.rpm ' + self.workDir)

    def Package(self):
        if (self.distro == 'Ubuntu'):
            self.MakeDebPackage()
        elif (self.distro == 'RHEL'):
            self.MakeRpmPackage()
        print('Package is generated successfully')

    def Release(self, tag):
        rpmPackageName = 'amdvlk-' + self.version + '-el.x86_64.rpm'
        debPackageName = 'amdvlk_' + self.version + '_amd64.deb';

        if not os.path.isfile(self.workDir + '/' + rpmPackageName):
            print('Can not find package: ' + rpmPackageName)
            sys.exit(-1)
        if not os.path.isfile(self.workDir + '/' + debPackageName):
            print('Can not find package: ' + debPackageName)
            sys.exit(-1)

        releaseNote = '[Driver installation instruction](https://github.com/GPUOpen-Drivers/AMDVLK#install-with-pre-built-driver) \n\n';
        formated_str = self.descript.replace("New feature and improvement", "## New feature and improvement")
        formated_str = formated_str.replace("Issue fix", "## Issue fix")
        releaseNote += formated_str;

        newRelease = self.repo.create_git_release(tag, tag, releaseNote, False, False);

        newRelease.upload_asset(self.workDir + '/' + rpmPackageName, rpmPackageName + '(RedHat 7 8)');
        newRelease.upload_asset(self.workDir + '/' + debPackageName, debPackageName + '(Ubuntu 18.04 20.04)');

        print("Released " + tag + " successfully")

    def start(self):
        self.GetOpt();
        self.ConnectGithub();
        self.GetReleasedTagsOnGithub();
        self.CloneAMDVLK();
        # Build and package if there is any tag un-released, only release the first found un-released tag.
        found = False;
        for tag in self.tagList:
            if tag not in self.relTagList:
                self.DownloadAMDVLKComponents();
                self.GetRevisions(tag);
                if (self.type == 'build'):
                    self.Build();
                    self.Package();
                elif (self.type == 'release'):
                    self.Release(tag);
                found = True;
                break

        if found == False:
            print("All of the tags are released!");

if __name__ == '__main__':
    worker = Worker();
    worker.start();

