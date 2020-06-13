@ECHO OFF

CD /D %~dp0

@ECHO "Create app service"
@ECHO sc create 服务名称 binPath="服务应用程序全路径 执行参数" start=Auto DisplayName="服务显示名称" 
sc create app binPath="%CD%\app.exe --service" start=Auto DisplayName="App Service" 

@ECHO "Configure app description service"
@ECHO sc description 服务名称 "设置服务描述"
sc description app "App Service"

@ECHO "Configure app service interact own"
@ECHO sc config 服务名称 type= interact type= own
sc config app type= interact type= own

@ECHO "Start app service"
@ECHO net start 服务名称
net start app
