@ECHO OFF

CD /D %~dp0

@ECHO "Create app service"
@ECHO sc create �������� binPath="����Ӧ�ó���ȫ·�� ִ�в���" start=Auto DisplayName="������ʾ����" 
sc create app binPath="%CD%\app.exe --service" start=Auto DisplayName="App Service" 

@ECHO "Configure app description service"
@ECHO sc description �������� "���÷�������"
sc description app "App Service"

@ECHO "Configure app service interact own"
@ECHO sc config �������� type= interact type= own
sc config app type= interact type= own

@ECHO "Start app service"
@ECHO net start ��������
net start app
