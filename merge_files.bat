
exit
echo cur_path:%~dp0
cd %~dp0tools\file_tool\ && filetool.exe --root_path=../../ --partition_targets=../../partition_file_list.file_list --function=merge --partition_size=104857600
exit

