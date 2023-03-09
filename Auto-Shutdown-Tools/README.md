# Auto Shutdown Tools

Auto Shutdown Tools provide a mechanism to automatically shutdown a computer if a user has not been logged in for more than an hour (or other configured time interval).

This has been written for use on cloud platforms (e.g., Azure) where you are billed for the time that the virtual machines are running. This is useful in training environments where the virtual machines are unlikely to need to run constantly and the users may not remember to stop them.

Note that when the virtual machine is shutdown on Azure it is not deallocated. This requires an additional step to be carried out either using a runbook or a server that is monitoring for stopped virtual machines. An example runbook can be found below.

## Windows

The Windows version of Auto Shutdown Tools has two components, a service that runs in the background, and a user-land application that can display a message on the desktop if the computer has reached the active timeout. The active timeout is how long you expect the virtual machine to run with a user logged in, by default this is 8 hours.

The Windows version of the Auto Shutdown Tools are provided as source code with the Visual Studio solution files.

### Prerequisites

The Auto Shutdown Tools Windows service requires the Visual C++ Runtime to be installed. This can be downloaded from [Microsoft here](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)

### Installation

The Auto Shutdown Tools Windows service is installed using the command:
```
AutoShutdownToolsService.exe install
```

The Auto Shutdown Tools UI can be run using a scheduled task when a user logs in:
```
schtasks /create /sc onlogon /tn AutoShutdownToolsUI /ru "BUILTIN\Administrators" /tr C:\Path\To\AutoShutdownToolsUI.exe /it /rl highest /delay 0001:00
```

### Registry Settings

The timeers can be modified at runtime using registry entries. The default values are:
```
[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\AutoShutdownToolsService\Parameters]
"Timeout"=dword:0000003C
"ShutdownOnStop"=dword:00000000
"ActiveTimeout"=dword:01877400
"ActiveTimeoutIncrement"=dword:0036EE80
"DebugLogging"=dword:00000000
```

| Timer | Description | Registry DWORD | Time |
| --- | --- | --- | --- |
| Timeout | How long to wait with no active session before shutdown | 0000003C | 60 minutes |
| ActiveTimeout | How long can the computer be running before checking if the user is still active | 01877400 | 8 hours (registry value in seconds) |
| ActiveTimeoutIncrement | How much time to add if the user clicks that they are active | 0036EE80 | 1 hour ( registry value in seconds) |

## Linux

The Linux version of the Auto Shutdown Tools are a shell script that is run as a service using systemd.

### Manual Installation

To install the Auto Shutdown Tools on Linux run: 
```
AutoShutdownTools.sh
```

This script will store the script in /usr/bin/AutoShutdownTools.sh and create the systemd file /lib/systemd/system/AutoShutdownTools.service.
These are stored base64 encoded in the installation script.

The script will make /usr/bin/AutoShutdownTools.sh executable, the enable and start the service using systemctl.

### Installation Through an ARM Template

The Auto Shutdown Tools on Linux can also easily be deployed as part of an ARM template when a new Linux VM is created by including a base64 encoded copy of the installation script. An example can be found in [Linux/ARM-Template-Example](Linux/ARM-Template-Example).

## Deallocating Stopped VMs in Azure

An example runbook to find and deallocate stopped VMs in Azure:
```
try
{
    Connect-AzAccount -Identity
}
catch {
    Write-Error -Message $_.Exception
    throw $_.Exception
}

Select-AzSubscription <SUBSCRIPTION ID>
 
[array]$AzVMs = Get-AzVM -Status
 
if($AzVMs) {
	ForEach($VmItem in $AzVMs) {
        if($VmItem.PowerState -eq "VM stopped") {
		    Stop-AzVM -Name $VmItem.Name -ResourceGroupName $VmItem.ResourceGroupName -Force
        }
   }
}
```
