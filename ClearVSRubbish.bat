echo off




rem �ݹ�ɾ����ǰ�ļ��У�".\"����"./"����ָ����չ�����ļ�


rem �����Ҫ�����������չ���ļ���������del��ͷ��ʽ���У�������Ӷ�Ӧ����չ������

rem ���del��չ����ע�⣺��"*.res"�����"*.resx"���ļ�Ҳɾ����c#���õ�"*.resx"�ļ�������

rem ��ͷ������Ϊע����䣬��������


set /p VAR=�Ƿ�ɾ��ָ�����ļ�����ѡ��:[Y, N]?



rem echo %VAR%



if /i '%VAR%' == 'y' goto s1


if /i '%VAR%' == 'n' goto end



:s1


for /R .\ %%i in (.\) do (cd %%i
	


	del /f /s /q *.sdf
	del /f /s /q *.pch

rem ������׼��ɾ����debug���͡�release���ļ��е����ݣ�������ɾ������


rem rd /s/q Release
rem rd /s/q Release_Unicode
rem rd /s/q Debug
rd /s/q ipch


)



:end


pause
