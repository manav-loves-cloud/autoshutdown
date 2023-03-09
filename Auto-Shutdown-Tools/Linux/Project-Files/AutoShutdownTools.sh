#!/bin/bash

##########################################################################################################################################
# Copyright 2022 Sophos Ltd.  All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing 
# permissions and limitations under the License.
##########################################################################################################################################

scriptCode="IyEvYmluL2Jhc2gKCiMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIwojIENvcHlyaWdodCAyMDIyIFNvcGhvcyBMdGQuICBBbGwgcmlnaHRzIHJlc2VydmVkLgojIExpY2Vuc2VkIHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZSAiTGljZW5zZSIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlIHdpdGggdGhlIExpY2Vuc2UuCiMgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0IGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMAojIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZywgc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW4gIkFTIElTIiBCQVNJUywgCiMgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyAKIyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnMgdW5kZXIgdGhlIExpY2Vuc2UuCiMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIwoKaWRsZUNvdW50ZXI9MApsb2dGaWxlPSIvdmFyL2xvZy9BdXRvU2h1dGRvd25Ub29scy5sb2ciCgp3aGlsZSB0cnVlCmRvCiAgICAgICAgc3NoU2Vzc2lvbnM9YHdobyB8IHdjIC1sYAogICAgICAgIGlmIFsgJHNzaFNlc3Npb25zIC1lcSAwIF0KICAgICAgICB0aGVuCiAgICAgICAgICAgICAgICBpZGxlQ291bnRlcj0kKChpZGxlQ291bnRlcisxKSkKICAgICAgICAgICAgICAgIGVjaG8gYGRhdGVgICIgTk8gU1NIIHNlc3Npb25zIERldGVjdGVkLCBpbmNyZW1lbnRpbmcgaWRsZUNvdW50ZXI6ICIgJGlkbGVDb3VudGVyID4+ICRsb2dGaWxlCiAgICAgICAgZWxzZQogICAgICAgICAgICAgICAgaWRsZUNvdW50ZXI9MAogICAgICAgICAgICAgICAgZWNobyBgZGF0ZWAgIiBTU0ggc2Vzc2lvbnMgZGV0ZWN0ZWQsIHJlc2V0dGluZyBpZGxlQ291bnRlcjogIiAkaWRsZUNvdW50ZXIgPj4gJGxvZ0ZpbGUKICAgICAgICBmaQoKICAgICAgICBpZiBbICRpZGxlQ291bnRlciAtZ3QgNjAgXQogICAgICAgIHRoZW4KICAgICAgICAgICAgICAgIGVjaG8gYGRhdGVgICIgKioqIFRpbWVvdXQgKCIgJGlkbGVDb3VudGVyICIpLCBzaHV0dGluZyBkb3duICoqKiIgPj4gJGxvZ0ZpbGUKICAgICAgICAgICAgICAgIHNodXRkb3duIC1oIG5vdwogICAgICAgIGZpCgogICAgICAgICMgV2FpdCBmb3IgMSBtaW51dGUgYmVmb3JlIG5leHQgY2hlY2sKICAgICAgICBzbGVlcCA2MApkb25lCgo="

systemdFile="IyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjCiMgQ29weXJpZ2h0IDIwMjIgU29waG9zIEx0ZC4gIEFsbCByaWdodHMgcmVzZXJ2ZWQuCiMgTGljZW5zZWQgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlICJMaWNlbnNlIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2Ugd2l0aCB0aGUgTGljZW5zZS4KIyBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXQgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wCiMgVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLCBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhbiAiQVMgSVMiIEJBU0lTLCAKIyBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTlkgS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gU2VlIHRoZSBMaWNlbnNlIGZvciB0aGUgc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIAojIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9ucyB1bmRlciB0aGUgTGljZW5zZS4KIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjCgpbVW5pdF0KRGVzY3JpcHRpb249TW9uaXRvciBmb3Igbm90IGFjdGl2ZSB1c2VyIGFuZCBzaHV0ZG93biB0aGUgY29tcHV0ZXIKCltTZXJ2aWNlXQpFeGVjU3RhcnQ9L3Vzci9iaW4vQXV0b1NodXRkb3duVG9vbHMuc2gKCltJbnN0YWxsXQpXYW50ZWRCeT1tdWx0aS11c2VyLnRhcmdldAo="

scriptPath="/usr/bin/AutoShutdownTools.sh"
systemdPath="/lib/systemd/system/AutoShutdownTools.service"

echo $scriptCode | base64 -d > $scriptPath
echo $systemdFile | base64 -d > $systemdPath

chmod +x $scriptPath

systemctl daemon-reload
systemctl enable AutoShutdownTools.service
systemctl start AutoShutdownTools.service

