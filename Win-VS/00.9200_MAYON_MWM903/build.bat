cd .\lib
move tutk.lib tutk
del *.lib
move tutk tutk.lib
cd ..\

cd .\dummy
make  -f dummy_Debug.mk
cd ..\

cd .\ucos_ii
make  -f ucos_ii_Release.mk
cd ..\

cd .\main
make  -f main_Release.mk
cd ..\

cd .\debug
make  -f debug_Debug.mk
cd ..\

cd .\fs
make  -f fs_Release.mk
cd ..\

cd .\shell
make  -f shell_Debug.mk
cd ..\

cd .\encrypt
make  -f encrypt_Debug.mk
cd ..\

cd .\uart
make  -f uart_Debug.mk
cd ..\

cd .\sdc
make  -f sdc_Debug.mk
cd ..\

cd .\usb
make  -f usb_Debug.mk
cd ..\

cd .\i2c
make  -f i2c_Debug.mk
cd ..\

cd .\jpeg
make  -f jpeg_Debug.mk
cd ..\

cd .\VideoCodec\common
make  -f VideoCodecCommon_Debug.mk
cd ..\..\

cd .\VideoCodec\H264
make  -f H264_Debug.mk
cd ..\..\

cd .\VideoCodec\mpeg4
make  -f mpeg4_Release.mk
cd ..\..\

cd .\siu
make  -f siu_Debug.mk
cd ..\

cd .\ramdisk
make  -f ramdisk_Debug.mk
cd ..\

cd .\hdmi
make  -f hdmi_Debug.mk
cd ..\

cd .\ipu
make  -f ipu_Debug.mk
cd ..\

cd .\isu
make  -f isu_Debug.mk
cd ..\

cd .\idu
make  -f idu_Debug.mk
cd ..\

cd .\timer
make  -f timer_Debug.mk
cd ..\

cd .\gpio
make  -f gpio_Debug.mk
cd ..\

cd .\rtc
make  -f rtc_Debug.mk
cd ..\

cd .\gpi
make  -f gpi_Debug.mk
cd ..\

cd .\Lwip
make  -f Lwip_Debug.mk
cd ..\

cd .\smc
make  -f smc_Debug.mk
cd ..\

cd .\iis
make  -f iis_Debug.mk
cd ..\

cd .\adc
make  -f adc_Debug.mk
cd ..\

cd .\ui
make  -f ui_Debug.mk
cd ..\

cd .\hiu
make  -f hiu_Debug.mk
cd ..\

cd .\dcf
make  -f dcf_Debug.mk
cd ..\

cd .\dpof
make  -f dpof_Debug.mk
cd ..\

cd .\sys
make  -f sys_Debug.mk
cd ..\

cd .\mp4
make  -f mp4_Debug.mk
cd ..\

cd .\asf
make  -f asf_Release.mk
cd ..\

cd .\avi
make  -f avi_Debug.mk
cd ..\

cd .\mov
make  -f mov_Release.mk
cd ..\

cd .\isp
make  -f isp_Debug.mk
cd ..\

cd .\spi
make  -f spi_Debug.mk
cd ..\

cd .\mars_system
make  -f mars_system_Debug.mk
cd ..\

cd .\mars_int
make  -f mars_int_Debug.mk
cd ..\

cd .\mars_dma
make  -f mars_dma_Debug.mk
cd ..\

cd .\mars_timer
make  -f mars_timer_Debug.mk
cd ..\

cd .\RFIU
make  -f RFIU_Debug.mk
cd ..\

cd .\CIU
make  -f CIU_Debug.mk
cd ..\

cd .\SDK
make  -f SDK_Debug.mk
cd ..\

cd .\board
make  -f board_Release.mk
cd ..\

