#!/usr/bin/env groovy

/**
 * Pipeline for building and upload deb package of AMDVLK
 *
 * Build parameters:
 *      @githubToken: token to access github
 */

def buildNode = "Ubuntu && 18-04 && vulkan"

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
        stage("BuildPackage") {
            agent {
                node { label params.buildNode }
            }
            steps {
                runScript()
            }
        }
    }
}

def runScript() {
    sh "python3 ${WORKSPACE}/utils/amdvlk_build_deb_from_tag.py -w ${WORKSPACE} -a ${githubToken}"
}
