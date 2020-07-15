#!/usr/bin/env groovy

/**
 * Pipeline for building and upload package of AMDVLK release
 *
 * Build parameters:
 *      @githubToken: token to access github
 */

def buildNode = "srdcvk && build && ubuntu"
def rpmPkgStash = "rpmStash"

pipeline {
    agent none
    parameters {
        string(
            name: "buildNode",
            defaultValue: params.buildNode ? params.buildNode : buildNode,
            description: "Jenkins node label or name of machine to run build stage on"
        )
    }
    stages {
        stage("Package") {
            parallel {
                stage("Ubuntu") {
                    agent { label "srdcvk && build && ubuntu" }
                    steps {
                        sh "rm -rf *.deb"
                        sh "python3 ${WORKSPACE}/amdvlk_release_for_tag.py -w ${WORKSPACE} -a ${githubToken} -t build"
                    }
                }
                stage("RHEL") {
                    agent { label "srdcvk && build && RHEL" }
                    steps {
                        script {
                            sh "rm -rf *.rpm"
                            sh "python3 ${WORKSPACE}/amdvlk_release_for_tag.py -w ${WORKSPACE} -a ${githubToken} -t build"
                            def pkgName = sh (script: "ls|grep *.rpm", returnStdout: true)
                            stash name: rpmPkgStash, includes: pkgName.trim()
                        }
                    }
                }
            }
        }
        stage("Release") {
            agent { label buildNode }
            steps {
                unstash name: rpmPkgStash
                sh "python3 ${WORKSPACE}/amdvlk_release_for_tag.py -w ${WORKSPACE} -a ${githubToken} -t release"
            }
        }
    }
}
