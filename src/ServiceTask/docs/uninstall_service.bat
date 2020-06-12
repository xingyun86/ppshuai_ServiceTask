@ECHO OFF

CD /D %~dp0

@ECHO "Stop app service"
net stop app

@ECHO "Delete app service"
sc delete app 

