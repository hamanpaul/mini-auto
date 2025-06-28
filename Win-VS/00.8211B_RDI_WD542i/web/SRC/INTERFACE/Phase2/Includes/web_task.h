extern		rpObjectDescriptionPtr gRpObjectList[];
extern      BOOLEAN_T	reboot_flag;
extern      BOOLEAN_T	start_count;
extern      UI32_T		SReboot_time;

void web_task(UNSIGNED argc, VOID *argv);
void WEB_Init(void);
void WEB_Terminate(void);
void web_task(UNSIGNED argc, VOID *argv);