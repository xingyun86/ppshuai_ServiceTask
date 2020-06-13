@ECHO OFF

CD /D %~dp0

@ECHO "Close app process"
TASKKILL /F /IM app.exe

@ECHO "Stop app service"
@ECHO net stop 服务名称
net stop app

@ECHO "Delete app service"
@ECHO sc delete 服务名称 
sc delete app 

