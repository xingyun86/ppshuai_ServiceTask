@ECHO OFF

CD /D %~dp0

@ECHO "Close app process"
taskkill /f /im app.exe

@ECHO "Stop app service"
@ECHO net stop ��������
net stop app

@ECHO "Delete app service"
@ECHO sc delete �������� 
sc delete app 

