pipeline {
  agent none
  stages {
    stage('Greeting') {
      steps {
        echo 'Hello Vulkan Open Source'
      }
    }
    stage('Builds') {
      parallel {
        stage('Build32') {
          steps {
            sh 'sfsdfsd'
          }
        }
        stage('Build64') {
          steps {
            sh 'Build64'
          }
        }
      }
    }
    stage('Tests') {
      steps {
        echo 'Testing starts'
      }
    }
    stage('Deploy') {
      steps {
        echo 'Deployment starts...'
      }
    }
  }
}