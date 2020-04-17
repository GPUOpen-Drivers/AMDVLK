#!/usr/bin/python3

# This script is used to build the AMD open source vulkan driver and make a deb package from github for tags.

# Before running this script, please install dependency packages with
# apt install repo python-pip python-git -y
# pip install PyGithub

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

class BuildDeb:
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

        parser.add_option("-t", "--targetRepo", action="store",
                          type="string",
                          dest="targetRepo",
                          help="Specify the target repo of github, default is " + self.targetRepo)

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

        if options.targetRepo:
            self.targetRepo = options.targetRepo;

        print("The target repos is " + self.targetRepo);

    def ConnectGithub(self):
        foundRepo = False;
        self.github = Github(self.accessToken);
        for repo in self.github.get_user().get_repos():
            if (repo.name == 'AMDVLK'):
                self.repo = repo;
                foundRepo = True;

        if (foundRepo == False):
            print("Fatal: AMDVLK repo is not found");
            sys.exit(-1);

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
        # build amdvlk64.so
        os.chdir(self.srcDir + 'xgl/');
        if os.system('cmake -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DBUILD_WAYLAND_SUPPORT=ON'):
            print("cmake -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release failed");
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
        if os.system('cmake -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=\"-Wno-error=unused-variable\"'):
            print("SPVGEN: cmake -H. -Brbuild64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=\"-Wno-error=unused-variable\" failed");
            exit(-1);

        os.chdir('rbuild64');
        if os.system('make -j8'):
            print("build spvgen failed");
            exit(-1);


    def Package(self):
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

    def UploadPackage(self, tag):
        releaseNote = '[Driver installation instruction](https://github.com/GPUOpen-Drivers/AMDVLK#install-with-pre-built-driver) \n\n';
        for line in self.descript:
            if line.strip() == 'New feature and improvement' or line.strip() == 'Issue fix':
                line = '## ' + line.strip() + '\n'
            releaseNote += line;

        newRelease = self.repo.create_git_release(tag, tag, releaseNote, False, False);
        packageName = 'amdvlk_' + self.version + '_amd64.deb';
        newRelease.upload_asset(self.workDir + '/' + packageName, packageName + '(Ubuntu 16.04 18.04)');

    def start(self):
        self.GetOpt();
        self.ConnectGithub();
        self.GetReleasedTagsOnGithub();
        self.CloneAMDVLK();
        # Build and package if there is any tag un-released.
        downloaded   = False;
        PackageCount = 0;
        for tag in self.tagList:
            if tag not in self.relTagList:
                if not downloaded:
                    self.DownloadAMDVLKComponents();
                    downloaded = True;
                self.GetRevisions(tag);
                self.Build();
                self.Package();
                self.UploadPackage(tag);
                PackageCount += 1;
                print("The package is generated successfully for " + tag);

        if PackageCount == 0:
            print("All of the tags are released!");

if __name__ == '__main__':
    buildDeb = BuildDeb();
    buildDeb.start();

