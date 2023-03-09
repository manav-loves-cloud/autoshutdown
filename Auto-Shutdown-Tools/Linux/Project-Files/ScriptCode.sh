#!/bin/bash

##########################################################################################################################################
# Copyright 2022 Sophos Ltd.  All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing 
# permissions and limitations under the License.
##########################################################################################################################################

idleCounter=0
logFile="/var/log/AutoShutdownTools.log"

while true
do
        sshSessions=`who | wc -l`
        if [ $sshSessions -eq 0 ]
        then
                idleCounter=$((idleCounter+1))
                echo `date` " NO SSH sessions Detected, incrementing idleCounter: " $idleCounter >> $logFile
        else
                idleCounter=0
                echo `date` " SSH sessions detected, resetting idleCounter: " $idleCounter >> $logFile
        fi

        if [ $idleCounter -gt 60 ]
        then
                echo `date` " *** Timeout (" $idleCounter "), shutting down ***" >> $logFile
                shutdown -h now
        fi

        # Wait for 1 minute before next check
        sleep 60
done

