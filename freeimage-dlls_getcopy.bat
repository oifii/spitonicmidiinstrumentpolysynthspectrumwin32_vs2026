:LABEL1
rem 2026jan15 freeimage_vs2026 instead
rem 32bit
rem 32bit
rem 32bit
copy ..\lib-src\freeimage_vs2026\Dist\x32\FreeImage.dll FreeImage.dll
copy ..\lib-src\freeimage_vs2026\Wrapper\FreeImagePlus\dist\x32\FreeImagePlusd.dll FreeImagePlusd.dll
copy ..\lib-src\freeimage_vs2026\Dist\x32\FreeImage.dll debug\FreeImage.dll
copy ..\lib-src\freeimage_vs2026\Wrapper\FreeImagePlus\dist\x32\FreeImagePlusd.dll debug\FreeImagePlusd.dll
rem release dlls
copy ..\lib-src\freeimage_vs2026\Dist\x32\FreeImage.dll release\FreeImage.dll
copy ..\lib-src\freeimage_vs2026\Wrapper\FreeImagePlus\dist\x32\FreeImagePlus.dll release\FreeImagePlus.dll


rem 64bit
rem 64bit
rem 64bit
copy ..\lib-src\freeimage_vs2026(x64)\Dist\x64\FreeImage.dll FreeImage.dll
copy ..\lib-src\freeimage_vs2026(x64)\Wrapper\FreeImagePlus\dist\x64\FreeImagePlus.dll FreeImagePlus.dll
copy ..\lib-src\freeimage_vs2026(x64)\Dist\x64\FreeImage.dll x64\debug\FreeImage.dll
copy ..\lib-src\freeimage_vs2026(x64)\Wrapper\FreeImagePlus\dist\x64\FreeImagePlus.dll x64\debug\FreeImagePlus.dll
rem release dlls
copy ..\lib-src\freeimage_vs2026(x64)\Dist\x64\FreeImage.dll x64\release\FreeImage.dll
copy ..\lib-src\freeimage_vs2026(x64)\Wrapper\FreeImagePlus\dist\x64\FreeImagePlus.dll x64\release\FreeImagePlus.dll
exit

