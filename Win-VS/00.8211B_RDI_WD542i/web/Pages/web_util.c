////////////////////////////////////////////////////////////////////////
// util.c
#include <stdio.h>
#include <stdarg.h>
#include "AsEngine.h"
#include "RomPager.h"
#include "web_util.h"

#define HTML_BUFFER_SIZE  1280
#define Port_in_Table_Max_length    55


char errCode[kNumberOfConnections][HTML_BUFFER_SIZE];
char szHtmlBuffer[HTML_BUFFER_SIZE];
int  nBuffUsed = 0;
char* pszHtmlBufferHead = &szHtmlBuffer[0];
char szHtmlBuffer_Menu[HTML_BUFFER_SIZE];
int  nBuffUsed_Menu = 0;
char* pszHtmlBufferHead_Menu = &szHtmlBuffer_Menu[0];
char *html_SetBuffer(char *);

int g_iFormSubmitCommand= 0;	// Apply(0), Refresh(1), Delete(2)
int g_iItemFrom= 0;//many file use

//===================== Error Message Table ============================
WEB_ERROR_TABLE ErrMegTable[]=
{
	//---------------------------- General Start----------------------------------
	{WEB_ERROR_NO_ERROR , " Success!"},
	{WEB_ERROR_GET_FAIL , "Fail!"},
	{WEB_ERROR_SET_ERROR , "Fail!"},
	{WEB_ERROR_ENTRY_NOT_EXIST , "Entry does not exist!"},
	{WEB_ERROR_ENTRY_ALREADY_EXIST , "Entry already exists!"},
	{WEB_ERROR_TABLE_FULL , "Table is Full!"},
	//---------------------------- General Stop ----------------------------------
	{WEB_ERROR_MIRROR_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_MIRROR_TARGETPORT_INVALID , " Target port cannot be trunk member!"},
	{WEB_ERROR_TRUNK_UPDATE_FAIL , "Update fail!"},
	{WEB_ERROR_TRUNK_GROUPNAME_INVALID , "Group name is invalid!"},
	{WEB_ERROR_TRUNK_GROUPID_INVALID , "Group ID is invalid!"},
	{WEB_ERROR_TRUNK_MASTERPORT_INVALID , "Master Port is invalid!"},
	{WEB_ERROR_TRUNK_OVERLAP_GROUP , "Config member ports are overlapped - Group!"},
	{WEB_ERROR_TRUNK_UNIT_GROUPNUM , "Too many groups in this slot!"},
	{WEB_ERROR_TRUNK_PORT_LINK_ERROR , "The members should not be in half duplex mode!"},
	{WEB_ERROR_TRUNK_PORT_TYPE_ERROR , "The members should be all with same type!"},
	{WEB_ERROR_TRUNK_MIRROR_ERROR , "Trunk Member cannot overlap the Mirror Target port!"},
	{WEB_ERROR_TRUNK_MBA_PORT_OVERLAP , "Some ports are MBA ports and can't be trunk ports!"},
	{WEB_ERROR_IS_NOT_MASTER_PORT , "Only trunk master can be configured!"},
	{WEB_ERROR_IGMP_VLAN_NOT_EXIST , "VLAN does not exist!"},
	{WEB_ERROR_IGMP_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_IGMP_TIMER_NOT_CHANGE , "Timer is followed IGMP setting, it cannot be changed."},
	{WEB_ERROR_STATIC_ROUTERPORT_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_FORWARD_UNI_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_FORWARD_UNI_VID_NOTEXIST , "VLAN ID does not exist!"},
	{WEB_ERROR_FORWARD_MULTI_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_FORWARD_MULTI_VID_NOTEXIST , "VLAN ID does not exist!"},
	{WEB_ERROR_FILTER_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_FILTER_ENTRY_HAD_EXIST , "MAC filter entry already exists!"},
	{WEB_ERROR_8021QVLAN_VID_ERROR , "Input VLAN ID is invaild!"},
	{WEB_ERROR_8021QVLAN_UNTAGE_OVERLAP , "Untagged ports are overlapped!"},
	{WEB_ERROR_8021QVLAN_VLAN_ID_EXIST , "VLAN ID already exists!"},
	{WEB_ERROR_ROUTER_MAC_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_ROUTER_MAC_ENTRY_HAD_EXIST , "Router MAC entry already exists!"},
	{WEB_ERROR_ROUTER_MAC_VID_NOT_EXIST , "VLAN ID does not exist!"},
	{WEB_ERROR_ROUTER_MAC_ENTRY_FULL , "Entry is Full!"},
	{WEB_ERROR_ROUTER_MAC_MAC_EXIST , "MAC Address already exists!"},
	{WEB_ERROR_ROUTER_MAC_MAC_NOT_FOUND , "MAC address is not found!"},
	{WEB_ERROR_CFGIP_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_CFGIP_VLAN_NOTEXIST , "VLAN does not exist!"},
	{WEB_ERROR_CFGIP_ROUTE_UPDATE_FAIL , "Route update fail!"},
	{WEB_ERROR_USER_ACCOUNT_UPDATE_FAIL , " fail!"},
	{WEB_ERROR_USER_ACCOUNT_NEED_ADMINUSER , "At least one admin user is needed!"},
	{WEB_ERROR_USER_ACCOUNT_USER_HAD_EXIST , " User account entry already exists!"},
	{WEB_ERROR_USER_ACCOUNT_USERNAME_INVALID , " User name error !"},
	{WEB_ERROR_FACTORYRESET_FINISH , "Finish !"},
	{WEB_ERROR_USER_ACCOUNT_ERR_DEL_USER_PWD , "Error: The password does not match."},
	{WEB_ERROR_USER_ACCOUNT_USER_TABLE_FULL , "No more user account can be added!"},
	{WEB_ERROR_IPADDR_INVALID,"Invalid IP Address!"},
	{WEB_USER_PWD_NOTMACTH,"Error: The Password does not match."},
    {WEB_USER_OLDPWD_NOTMACTH,"The old password error!"},
    {WEB_DEVICE_BUSY,"device busy !"},
	{WEB_ACL_PROFILE_ID_NOTEXIST,"Profile Id does not exist!"},
	{WEB_ACL_ICMP_VALUE_ERROR,"ICMP value Error (0-255)!"},
    {WEB_ACL_IGMP_VALUE_ERROR,"IGMP value Error (0-255)! "},
    {WEB_ACLRULE_SOURPORT_VALUE_ERROR,"Source port  value  Error !(0-65535)."},
    {WEB_ACLRULE_DESTPORT_VALUE_ERROR,"Dest port  value  Error !(0-65535)."},
	{WEB_ACLRULE_DSCP_VALUE_ERROR,"DSCP  value  Error !(0-63)."},
	{WEB_ACLRULE_P8021P_VALUE_ERROR, "8021P  value  Error !(0-7)."},
	/* ACL end */
    {WEB_ERROR_PORTLOCK_NOT_EXIST,"The port number does not exist."},
	{WEB_ERROR_PORTLOCK_NOT_ENABLE,"The port does not enable port_security."},
    {WEB_ERROR_PORTLOCK_NOT_CORRECT,"Port_security type is not correct."},
    {WEB_ERROR_IPADDR_NOT_BOOT_MODE,"When more than two IP interface entries exist,\\n it's invalid to change boot mode"},
	//----------------------- L3 Start ---------------------------
	{WEB_ERROR_L3_CFGMD5_KEYINDEX_EXIST , "Key Index entry already exists!"},
	{WEB_ERROR_L3_CFGMD5_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_CFGRTRED_ENTRY_EXIST , "Entry already exists!"},
	{WEB_ERROR_L3_CFGRTRED_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_IGMPIF_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_IGMPIF_GET_FAIL , "Fail in getting entryl!"},
	{WEB_ERROR_L3_IPIF_GET_FAIL , "Fail in getting entryl!"},
	{WEB_ERROR_L3_IPIF_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_IPIF_VLAN_NOTEXIST , "The VLAN name you entered does not exist."},
	{WEB_ERROR_L3_IPIF_ENTRY_EXIST , "Entry already exists!"},
	{WEB_ERROR_L3_IPIF_INVALID_HOST_ADDRESS, "Invalid IP Address!"},
	{WEB_ERROR_L3_DVMRPIF_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_DVMRPIF_GET_FAIL , "Entry not found!"},
	{WEB_ERROR_L3_PIMDMIF_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_PIMDMIF_GET_FAIL , "Entry not found!"},
	{WEB_ERROR_L3_GLOBALCFG_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_CFGAREAID_EXIST , "Area ID already exists!"},
	{WEB_ERROR_L3_CFGAREAID_UPDATE_FAIL , "Fail!"},
	{WEB_ERROR_L3_INVALID_INTERFACE_NAME, "Invalid interface name!"},
	{WEB_ERROR_L3_SECONDARY_IP_NOT_SUPPORT_BPR, "Secondary IP interface does not support bootp relay!"},
	{WEB_ERROR_L3_IGMP_ENABLE_ERROR,"Please disable IGMP Snooping Fast Leave before enabling IGMP."},
	{WEB_ERROR_L3_IGMPFASTLEAVE_ENABLE_ERROR,"Cannot set Fast leave when IGMP is running."},//Ben ,iris chu 04443 modify 2004/10/12 05:27¤U¤È
	{WEB_ERROR_L3_RIPV1_NOT_SUPPORT_AUTHENTICATION,"RIP v1 does not support authentication."},
	{WEB_ERROR_L3_IPIF_ARP_ON_IPIF, "Delete IP Interface fail due to static ARP on this interface."},
	{WEB_ERROR_L3_IPIF_SETTING_NOTE, "Success! Note: All setting on this interface will return to default setting."},
	// ----------------------- L3 End --------------------------------
	// ---------------- ACL Start --------------------------------
	{WEB_ERROR_ACL_ID_EXIST,"Error ! Profile id already exists "},
	{WEB_ERROR_ACL_PROFILE_ID_NOTEXIST,"Profile Id does not exist!"},
	{WEB_ERROR_ACL_ICMP_VALUE_INVALID,"ICMP value error (0-255)!"},
	{WEB_ERROR_ACL_IGMP_VALUE_INVALID,"IGMP value error (0-255)! "},
	{WEB_ERROR_ACL_RULE_SOURPORT_VALUE_INVALID,"Source port  value  error! (0-65535)."},
	{WEB_ERROR_ACL_RULE_DESTPORT_VALUE_INVALID,"Dest port  value  error! (0-65535)."},
	{WEB_ERROR_ACL_RULE_DSCP_VALUE_INVALID,"DSCP  value  error! (0-63)."},
	{WEB_ERROR_ACL_RULE_P8021P_VALUE_INVALID, "8021P  value  error! (0-7)."},
	{WEB_ERROR_ACL_SET_RULE_NOTEXIST, "ACL Rule does not exist."},
	{WEB_ERROR_ACL_SET_RULE_FULL, "ACL Rule is full."},
	{WEB_ERROR_ACL_SET_RULE_PARTIAL_FAIL, "ACL Rule fails partially."},
	{WEB_ERROR_ACL_IF_ERR_NOT_MEMBER, "Fail!"},              /* rule port is not a member of profile */
	{WEB_ERROR_ACL_IF_ERR_PORT_INCONSISTENT, "Fail!"},
	{WEB_ERROR_ACL_IF_ERR_RULEBODY_DUPLICATED, "ACL Rule Body is Duplicated!"},      /* RULE BODY is duplicated even difference access id */
	{WEB_ERROR_ACL_ENTRY_NOT_OWNED_BY_ACL, "Success! Note: Some entries not owned by ACL will not be removed! "},
	{WEB_ERROR_ACL_METERING_FULL, "ACL Metering is full."},
    {WEB_ERROR_ACL_NOT_CREATED_BY_ACL, "This entry was not created by ACL."},
    {WEB_ERROR_SOFT_ACL_TABLE_FULL,"Table full!"},
	// ---------------- ACL End --------------------------------
	// ---------- SNMP V3 start ------------------------
	{WEB_ERROR_AuthProtocol_PWD, "Error ! Auth-Protocol Password length need 8-16"},
	{WEB_ERROR_PrivProtocol_PWD, "Error ! Priv-Protocol Password length need 8-16"},
	{WEB_INVALID_HOST_ADDRESS, "Error ! Invalid Host Address"},
	{WEB_BAD_MASK, "Error ! Invalid Mask"},
	{WEB_SNMP_HOST_EXIST, "The specified host already exists!"},
	{WEB_SNMP_COMMUNITY_VIEWNAME_NOTEXIST, "The View Name doesn't exist!"},
	{WEB_SNMP_COMMUNITY_NAME_EXIST, "The community already exits!"},
	{WEB_SNMP_USER_GROUPNAME_NOT_EXIST, "The specified group does not exist!"},
	{WEB_SNMP_USER_USERNAME_EXIST, "The user name already exists!"},
	{WEB_SNMP_VIEW_EXIST, "The specified view already exists!"},
	{WEB_SET_SNMP_VIEW_FULL, "The SNMP View table is full! "},//04443 modify 04-10-2003
	{WEB_SET_SNMP_COMMUNITY_FULL, "The SNMP Community table is full! "},//04443 modify 04-10-2003
	{WEB_SET_SNMP_GROUP_FULL, "The group table is full! "},
	{WEB_SET_SNMP_USER_FULL, "The user table is full! "},
	{WEB_SET_SNMP_HOST_FULL, "The host table is full! "},
	// ---------- SNMP V3 end ------------------------
	// ---------- FS Command start -------------------
	{WEB_ERROR_FS_INVALID_DRV_NAME, "Invalid drive name!"},
	{WEB_ERROR_FS_SAME_FILE, "The same filename has existed!"},
	{WEB_ERROR_FS_ROOT_DISKFULL, "Cannot create any file in root area!"},
	{WEB_ERROR_FS_DISKFULL, "Disk Full!"},
	{WEB_ERROR_FS_INVALID_DIR, "Invalid directory name."},
	{WEB_ERROR_FS_FILE_RO, "Cannot erase this directory due to existing files."},
	{WEB_ERROR_FS_NO_STORAGE, "No storage media!"},
	{WEB_ERROR_FS_FSTYPE_ERROR, "File system type doesn't match media size!"},
	{WEB_ERROR_FS_ERROR_INPUT, "Input error format!"},
	{WEB_ERROR_FS_SAME_DIR, "The same directory already exists!"},
	{WEB_ERROR_FS_NO_FILE_DIR, "No such file or directory!"},
	{WEB_ERROR_FS_NO_FORMAT, "Storage media unformatted or not present!"},
	{WEB_ERROR_FS_ERROR_CHAR, "Invalid character entry!"},
	{WEB_ERROR_FS_ERROR_FILENAME, "Filename is too long!"},
	{WEB_ERROR_FS_EXIST_FILE_INDIR, "Some files exists in directory!"},
	{WEB_ERROR_FS_INPUT_MEDIA_ID, "Input Media ID Error!"},
	{WEB_ERROR_FS_CF_BUSY, "Fail!! CF busy!"},
	{WEB_ERROR_FS_DEVICE_BUSY, "Device busy!"},
	// ---------- FS Command end ---------------------
	//----------- VRRP start -------------------------
	{WEB_ERROR_VRRP_INVALID_IP, "Incorrect value of VR Ip!"},
	{WEB_ERROR_VRRP_ENTRY_FULL, "VRRP Router entries full!"},
	{WEB_ERROR_VRRP_ROUTER_EXISTS, "VRRP Router exists!"},
	{WEB_ERROR_VRRP_SET_ERROR, "VRRP Router vreation failed!"},
	{WEB_ERROR_VRRP_SET_IP_ERROR, "Configuration of  VRRP Router Ip failed!"},
	{WEB_ERROR_VRRP_CANT_CONFIG, "Ip Owner can't configure the preempt mode!"},
	{WEB_ERROR_VRRP_NO_ENTRY, "No entry for the ipif!"},
	{WEB_ERROR_VRRP_SET_CRITICAL_ERROR, "Configuration of Critical Ip failed!"},
	{WEB_ERROR_VRRP_ENABLE_CRITICAL_ERROR, "Enabling the Critical Ip failed!"},
	{WEB_ERROR_VRRP_SET_AUTH_FAIL, "Authentication configuration failed!"},
	//----------- VRRP end ---------------------------
	/* ------------------- AAA start --------------------------------- */
	{WEB_ERROR_AAA_ENTRY_NOT_EXIST, "The entry does not exist."},
	{WEB_ERROR_AAA_ENTRY_ALREADY_EXIST, "The entry already exists."},
	{WEB_ERROR_AAA_EEPROM_IS_FULL, "The EEPROM is full!"},
	{WEB_ERROR_AAA_CONSOLE_LOGIN_METHOD_LIST_NOT_EXIST, "Login method list for console does not exist."},
	{WEB_ERROR_AAA_TELNET_LOGIN_METHOD_LIST_NOT_EXIST, "Login method list for telnet does not exist."},
	{WEB_ERROR_AAA_SSH_LOGIN_METHOD_LIST_NOT_EXIST, "Login method list for ssh does not exist."},
	{WEB_ERROR_AAA_HTTP_LOGIN_METHOD_LIST_NOT_EXIST, "Login method list for http does not exist."},
	{WEB_ERROR_AAA_CONSOLE_ENABLE_METHOD_LIST_NOT_EXIST, "Enable method list for console does not exist."},
	{WEB_ERROR_AAA_TELNET_ENABLE_METHOD_LIST_NOT_EXIST, "Enable method list for telnet does not exist."},
	{WEB_ERROR_AAA_SSH_ENABLE_METHOD_LIST_NOT_EXIST, "Enable method list for ssh does not exist."},
	{WEB_ERROR_AAA_HTTP_ENABLE_METHOD_LIST_NOT_EXIST, "Enable method list for http does not exist."},
	{WEB_ERROR_AAA_DEFAULT_LOGIN_METHOD_LIST_CANNOT_DELETE, "Default login method list can not be deleted!"},
	{WEB_ERROR_AAA_DEFAULT_ENABLE_METHOD_LIST_CANNOT_DELETE, "Default enable method list can not be deleted!"},
	{WEB_ERROR_AAA_FIRST_METHOD_CANNOT_EMPTY, "The first method in the method list must not be empty."},
	{WEB_ERROR_AAA_USER_DEFINED_SERVER_GROUP_NOT_EXIST, "User-defined server group doesn't exist! Please create it first."},
	{WEB_ERROR_AAA_BUILT_IN_TACACS_SERVER_GROUP_NOT_EXIST, "Built-in server group TACACS doesn't exist!"},
	{WEB_ERROR_AAA_BUILT_IN_XTACACS_SERVER_GROUP_NOT_EXIST, "Built-in server group XTACACS doesn't exist!"},
	{WEB_ERROR_AAA_BUILT_IN_TACACS_PLUS_SERVER_GROUP_NOT_EXIST, "Built-in server group TACACS+ doesn't exist!"},
	{WEB_ERROR_AAA_BUILT_IN_RADIUS_SERVER_GROUP_NOT_EXIST, "Built-in server group RADIUS doesn't exist!"},
	{WEB_ERROR_AAA_TACACS_SERVER_HOST_NOT_EXIST, "No any TACACS server exists! Please create it first."},
	{WEB_ERROR_AAA_XTACACS_SERVER_HOST_NOT_EXIST, "No any XTACACS server exists! Please create it first."},
	{WEB_ERROR_AAA_TACACS_PLUS_SERVER_HOST_NOT_EXIST, "No any TACACS+ server exists! Please create it first."},
	{WEB_ERROR_AAA_RADIUS_SERVER_HOST_NOT_EXIST, "No any RADIUS server exists! Please create it first."},
	{WEB_ERROR_AAA_ANY_SERVER_HOST_NOT_EXIST, "No any server exists! Please create it first."},
	{WEB_ERROR_AAA_HOST_IN_USER_DEFINED_SERVER_GROUP_NOT_EXIST, "No any server exists in the User-defined server group."},
	{WEB_ERROR_AAA_HOST_IN_TACACS_SERVER_GROUP_NOT_EXIST, "No any server exists in the built-in server group TACACS."},
	{WEB_ERROR_AAA_HOST_IN_XTACACS_SERVER_GROUP_NOT_EXIST, "No any server exists in the built-in server group XTACACS."},
	{WEB_ERROR_AAA_HOST_IN_TACACS_PLUS_SERVER_GROUP_NOT_EXIST, "No any server exists in the built-in server group TACACS+."},
	{WEB_ERROR_AAA_HOST_IN_RADIUS_SERVER_GROUP_NOT_EXIST, "No any server exists in the built-in server group RADIUS."},
	{WEB_ERROR_AAA_INVALID_METHOD_NAME, "Method name is invalid for the method list."},
	{WEB_ERROR_AAA_INVALID_SERVER_GROUP_NAME, "Server group name is invalid."},
	{WEB_ERROR_AAA_SPECIFIC_SERVER_HOST_NOT_EXIST, "Server host doesn't exist! You need to create it first."},
	{WEB_ERROR_AAA_SERVER_HOST_ALREADY_IN_SERVER_GROUP, "Server group already has this server host!"},
	{WEB_ERROR_AAA_FULL_SERVER_HOST_IN_SERVER_GROUP, "Server group is full(max. 8 servers)! You need to remove some hosts out of it first."},
	{WEB_ERROR_AAA_DUPLICATE_METHOD_IN_A_METHOD_LIST, "Each Method in a method list must be unique."},
	{WEB_ERROR_AAA_BUILT_IN_SERVER_GROUP_ACCEPT_SERVER_HOST_WITH_SAME_PROTOCOL, "Built-in server group accepts the server host with the same protocol!"},
	{WEB_ERROR_AAA_SERVER_HOST_CREATE_SUCCESS_SERVER_GROUP_ADD_FAIL, "This server is created successfully. But this server can not be added into the built-in server group because the built-in server group is full(max. 8 servers)! If you want this server to do authentication, please add it into a user-defined server group."},
	{WEB_ERROR_AAA_SERVER_HOST_TABLE_FULL, "Server host table full! (Max. 16)"},
	{WEB_ERROR_AAA_SERVER_HOST_IP_CAN_NOT_BE_LOCAL_INTERFACE, "Server host ip can not be local interface !"},
	{WEB_ERROR_AAA_SERVER_GROUP_TABLE_FULL, "Server group table full! (Max. 8)"},
	{WEB_ERROR_AAA_ENTRY_ALREADY_EXISTS, "The Entry already exists!"},
	/* ------------------- AAA end -------------------------------- */
	/* ------------------- SSL start ------------------------------ */
	{WEB_ERROR_SSL_CERT_DOWNLOAD_RESULT_TFTP_FAIL, "TFTP failed!"},
	{WEB_ERROR_SSL_CERT_DOWNLOAD_RESULT_DECODE_FAIL, "Certificate Decode Failure!"},
	/* ------------------- SSL end -------------------------------- */
	/* ------------------- 802.1x start ------------------------------ */
	{WEB_ERROR_1X_GET_STATUS_FAIL, "Get 802.1X status failed!"},
	{WEB_ERROR_1X_GET_MODE_FAIL, "Get PAE authentication mode failed!"},
	{WEB_ERROR_1X_STATUS_NOT_ENABLE, "You must enable 802.1x first!"},
	{WEB_ERROR_1X_INVALID_MAC_ADDRESS, "Invalid MAC address!"},
	/* ------------------- 802.1x end -------------------------------- */
	/* ------------------- STP start --------------------------------- */
	{WEB_ERROR_STP_NOT_MASTER, "STP configuration can be only done at Master unit."},
	{WEB_ERROR_STP_FAIL, "STP Config fail!"},
	{WEB_ERROR_STP_SUCCESS, "STP Config success!"},
	{WEB_ERROR_STP_MSTI_ID_NOT_EXIST, "This MST instance does not exist."},
	{WEB_ERROR_STP_MSTI_ID_ALREADY_EXIST, "This MST instance already exists."},
	{WEB_ERROR_STP_MSTI_TABLE_FULL, "MST instance has already reached its maximum capacity"},
	{WEB_ERROR_STP_MSTI_VID_OCCUPIED_BY_OTHER_MSTI, "Mapped VLAN ID is already occupied by other MSTI."},
	{WEB_ERROR_STP_MSTI_VID_OUT_OF_RANGE, "VLAN ID should be between 1 and 4094."},
	{WEB_ERROR_STP_MSTI_NO_BRIDGE_HELLOTIMEv, "Can not create MSTP in 1w mode."},
	{WEB_ERROR_STP_MSTI_NO_PORT_HELLOTIME, "Can not configure Bridge HelloTime in 1s mode."},
	{WEB_ERROR_STP_BRIDGE_PRIORITY_INVALID, "Invalid Bridge Priority! Please input a number from 0 to 61440."},
	{WEB_ERROR_STP_BRIDGE_PRIORITY_GRANULARITY_INVALID, "The Bridge priority value must be divisible by 4096."},
	{WEB_ERROR_STP_BRIDGE_VERSION_INVALID, "Invalid Protocol version!"},
	{WEB_ERROR_STP_BRIDGE_MAXAGE_INVALID, "Invalid Max Age time! Please input a number from 6 to 40."},
	{WEB_ERROR_STP_BRIDGE_MAXHOPS_INVALID, "Invalid Max Hops! Please input a number from 1 to 10."},
	{WEB_ERROR_STP_BRIDGE_FWDDELAY_INVALID, "Invalid Forward Delay time! Please input a number from 4 to 30."},
	{WEB_ERROR_STP_BRIDGE_FWDDELAY_AND_MAXAGE_INCONSISTENT, "Forward Delay and Max Age time are inconsistent."},
	{WEB_ERROR_STP_BRIDGE_HELLOTIME_AND_MAXAGE_INCONSISTENT, "Hello Time and Max Age time are inconsistent.!"},
	{WEB_ERROR_STP_BRIDGE_TXHOLDCOUNT_INVALID, "Invalid TxHoldCount! Please input a number from 1 to 10."},
	{WEB_ERROR_STP_BRIDGE_HAS_THE_SAME_VALUE, "Configure value is the same with current value."},
	{WEB_ERROR_STP_STP_ALREADY_ENABLED, "The current STP state is already enabled."},
	{WEB_ERROR_STP_STP_ALREADY_DISABLED, "The current STP state is already disabled."},
	{WEB_ERROR_STP_PORT_INVALID, "Port number is invalid!"},
	{WEB_ERROR_STP_PORT_STACKING, "Please do not configure stacking port!"},
	{WEB_ERROR_STP_PORT_PRIORITY_INVALID, "Invalid port priority! Please input a number  from 0 to 240!"},
	{WEB_ERROR_STP_PORT_PRIORITY_GRANULARITY_INVALID, "The Port priority value must be divisible by 16."},
	{WEB_ERROR_STP_PORT_HELLOTIME_INVALID, "Invalid Hello Time! Please input a number from 1 to 10."},
	{WEB_ERROR_STP_PORT_PATHCOST_INVALID, "Invalid port path cost! Please input a number from 1 to 200000000 or auto."},
	{WEB_ERROR_STP_PORT_TRUNK_MEMBER_PORT, "Please don't configure Trunk member ports!"},
	{WEB_ERROR_STP_PORT_1DPATHCOST_INVALID, "Invalid port path cost! Please input a number from 1 to 65535 or auto"},
	/* ------------------- STP end ----------------------------------- */
        /* ------------------- IPBIND START ------------------------------ */
        /* Shanlu 94/05/26 */
	{WEB_ERROR_IP_MAC_BINDING_PORT_SEC, "port security is enabled on some ports!"},
	{WEB_ERROR_IP_MAC_BINDING_TRUNK, "TRUNK is enabled on some ports!"},
	{WEB_ERROR_IP_MAC_BINDING_IP_FAIL, "IP Fail!"},
	{WEB_ERROR_IP_MAC_BINDING_MAC_FAIL, "Mac Fail!"},
	{WEB_ERROR_IP_MAC_BINDING_STATIC_ARP, "IP/MAC binding with same MAC is enabled on some ports .!"},
	{WEB_ERROR_IP_MAC_BINDING_STATIC_MAC, "IP/MAC binding with same MAC is enabled on some ports!"},
        /* ------------------- IPBIND end -------------------------------- */
	/*-------------------- MAC Based Authentication START--------------*/
	{WEB_ERROR_MBA_PORTSECURITY, "You must disable port security first!"},
	{WEB_ERROR_MBA_AUTH_STATUS_FAIL, "Getting user-auth status failed!"},
	{WEB_ERROR_MBA_FAILURE, "Failure!"},
	{WEB_ERROR_MBA_PORT_CONFLICT,"Some ports are trunk ports, stacking ports, 8021x auth. ports, Guest VLAN ports, port security ports, GVRP enabled ports or Web based auth. ports, these ports can not be authenticators!"},
	{WEB_ERROR_MBA_GUEST_VLAN_INEXISTENT, "The guest VLAN is inexistent!"},
	{WEB_ERROR_MBA_GVRP_INFO_ERROR, "Failure to get GVRP port info!"},
	{WEB_ERROR_MBA_DOUBLE_VLAN_MODE_ENABLED, "Double vlan mode is enabled!"},
	{WEB_ERROR_MBA_VLAN_CONFLICT, "The authenticated port only can belong to one VLAN!"},
	{WEB_ERROR_MBA_RELATIVE_VLAN_INEXISTENT, "The relative VLAN is inexistent!"},
	{WEB_ERROR_MBA_ENTRY_EXISTENT, "MAC-port entry already exists!"},
	{WEB_ERROR_MBA_TABLE_FULL, "Table is full!"},
	{WEB_ERROR_MBA_ENTRY_INEXISTENT, "MAC-port entry doesn't exist!"},
	{WEB_ERROR_MBA_ENTRY_DELETE_FAIL, "Failure to delete MAC-port entry!"},
	{WEB_ERROR_MBA_GVRP_ENABLE, "GVRP enabled ports can not be MAC-based Auth. ports!"},
	{WEB_ERROR_MBA_WAU_GVLAN_ENABLE, "WEB-based Auth. ports and 802.1x guest VLAN ports can not be MAC-based Auth. ports!"},
	{WEB_ERROR_MBA_GVRP_WAU_GVLAN_ENABLE, "GVRP enabled ports, WEB-based Auth. ports and 802.1x guest VLAN ports can not be MAC-based Auth. ports!"},
	{WEB_ERROR_MBA_VLAN_INEXISTENT, "The VLAN is inexistent!"},
	{WEB_ERROR_MBA_INVALID_PASSWORD, "Invalid password!"},
	{WEB_ERROR_GVRP_MBA_PORT, "Some ports are MBA ports, and can not be modified!"},
	{WEB_ERROR_GVRP_MBA_GUEST_VLAN_PORT, "Some ports are MBA guest VLAN ports, and can not be modified!"},
	/*-------------------- MAC Based Authentication END--------------*/
	/* --------------------MISC START--------------------------------*/
	{WEB_ERROR_WAU_INTERFACE_DISBALE, "Web based auth interface can not be disabled !"},
	{WEB_ERROR_WAU_GVLAN_INCLUDE, "Some ports are web-based auth ports or guest VLAN ports and cannot be modified!"},
	{WEB_ERROR_WAU_INCLUDE, "some ports are web based auth ports and can not be modified !"},
	{WEB_ERROR_GVLAN_INCLUDE, "some ports are guest VLAN ports and can not be modified ! !"},
	{WEB_ERROR_WAU_VLAN, "This VLAN is web based authenticated VLAN and can not be deleted!"},
	{WEB_ERROR_GVLAN_VLAN, "This VLAN is guest VLAN and can not be deleted ! !"},
	{WEB_ERROR_WAU_INTERFACE_DELETE, "Web based auth interface can not be deleted !"},
	{WEB_ERROR_WAU_ENABLE, "Web based auth enabled and  SSL cannot be enabled!"},
	{WEB_ERROR_IP_MAC_BINDING_ACL_MASK_FAIL, "Warning! Not enough access profiles present to enable IPBIND & ACL state."},
	{WEB_ERROR_IP_MAC_BINDING_SET_PORT_ACL_RULE_FAIL, "Warning! Access profile entries are not configured properly for ports. Please reconfigure the access profile entries and port state."},
	{WEB_ERROR_IP_MAC_BINDING_ACL_RULE_FAIL, "Warning! Access profile entries are not configured properly.The other entries are inactive!!!"},
	{WEB_ERROR_SNOOP_RET_CONFLICT_VLAN, "The VLAN already exists in VLAN module."},
	{WEB_ERROR_SNOOP_RET_DUPLICAT_ENTRY, "Duplicate entry."},
	{WEB_ERROR_SNOOP_RET_TABLE_FULL, "The table is full."},
	{WEB_ERROR_SNOOP_RET_OUT_RANGE, "The VLAN ID is out of range."},
	{WEB_ERROR_SNOOP_RET_OVERLAP, "The member or source ports are overlaped inside a Multicast VLAN or among multicast VLANs."},
	{WEB_ERROR_SNOOP_RET_NOT_EXIST, "The multicast VLAN entry doesn't exist."},
   	{WEB_ERROR_PORT_PARTIAL_SETTING_FAIL, "Warning! Setting for some port(s) failed"},
   	{WEB_ERROR_DEL_CFG_DOESNT_EXIST, "Config. does not exist!"},
   	{WEB_ERROR_DEL_CFG_INVALID_ID, "Invalid Config. Id!"},
   	{WEB_ERROR_DEL_CFG_IS_ACTIVE, "Active Config. can not be deleted!"},
   	{WEB_ERROR_DEL_CFG_ERASE_ERROR, "Error on erasing Config.!"},
	/* --------------------MISC end--------------------------------*/
	/* --------------------802.1V START--------------------------- */
	{WEB_ERROR_8021V_GROUP_ID_FAIL,"The protocol group id out of range!"},
	{WEB_ERROR_8021V_GROUP_DB_FULL,"Protocol Group Database is Full!!!"},
	{WEB_ERROR_8021V_DEUPLICATE_GROUP,"Duplicated 1v group entry!!!"},
	{WEB_ERROR_8021V_NO_GROUP_ENTRY,"No such 1v group entry!!!"},
	{WEB_ERROR_8021V_INEXITENT_ENTRY,"Inexistent 1v group entry!!!"},
	{WEB_ERROR_8021V_PROTOCOL_SIZE_FULL,"The size of protocol templates in a group or a port is full"},
	{WEB_ERROR_8021V_PROTOCOL_EXISTED,"Configured protocol template already existed!!!"},
	{WEB_ERROR_8021V_PROTOCOL_INEXISTENT,"Configured protocol template inexistent!!!"},
	{WEB_ERROR_8021V_PROTOCOL_INVALID,"Configured protocol template invalid!!!"},
	{WEB_ERROR_8021V_GROUP_ID_EMPTY,"802.1v Protocol GroupID is empty!"},
	{WEB_ERROR_8021V_VLAN_NOT_EXIST,"The vlan entry is not exist!"},
    {WEB_ERROR_8021V_OVERLAPPED,"Overlapping happened in the assigned port with the same group id but differnt vids"},
    {WEB_ERROR_8021V_VLAN_IS_PROTOCOL_VLAN,"This vlan is Protocol VLAN, it can not be deleted."},
    {WEB_ERROR_8021V_LLC_ENTRY_FULL,"The size of LLC protocol templates is full."},
    {WEB_ERROR_8021V_LLC_USERDEF_ENTRY_FULL,"The size of LLC user define protocol templates is full."},
    {WEB_ERROR_8021V_LLC_DEFSEL_ENTRY_FULL,"The size of LLC default selection protocol templates is full."},
    /* --------------------802.1V END----------------------------- */
	/* --------------------MLD Snooping start------------------- */
    {WEB_ERROR_MLD_VLAN_NAME_ERROR , "The VLAN name you entered does not exist."},
    {WEB_ERROR_MLD_VLAN_NOT_EXIT,"The VLAN of the router-port entry does not exist."},
    {WEB_ERROR_MLD_CANNOT_CHANGE," Timer is followed MLD setting, it cannot be changed."},
    {WEB_ERROR_MLD_MLD_ENABLED,"MLD Status is Enabled!!."},
    {WEB_ERROR_MLD_SET_FASTDONE,"Cannot set Fast done while MLD is running."},
    {WEB_ERROR_MLD_TRUNK_FAILED," Trunk failed."},
    {WEB_ERROR_MLD_PORT_NOT_EXIST," This port doesn't exist in Router Portlist."},
    {WEB_ERROR_MLD_ROUTER_PORT_EXIST," This deleted port exists in MLD SNOOPING Router Portlist."},
    {WEB_ERROR_MLD_ROUTER_PORT_NOT_VLAN_MEMBER," The added port doesn't exist in vlan member Portlist."},
	/* --------------------MLD Snooping end--------------------- */
    /* --------------------check trunk start------------------- */
    {WEB_ERROR_CHECK_TRUNK_PORTLIST_FAIL, "TRUNK is enabled on some ports!"},
    /* --------------------check trunk end------------------- */
    /* --------------------loopback interface start------------------- */
    {WEB_ERROR_VIR_IN_NAME_EXIST, "The interface name already exists. Enter a unique interface name."},
    {WEB_ERROR_VIR_IN_NO_IP_EXIST, "No IP Interface exists."},
    {WEB_ERROR_VIR_IN_INV_ADDR, "Invalid host address assigned!"},
    {WEB_ERROR_VIR_IN_SYS_NOT_DEL, "The system IP interface can't be deleted."},
    {WEB_ERROR_VIR_IN_NAME_NOT_EXIST, "The VLAN name you entered does not exist."},
    {WEB_ERROR_VIR_IN_DEL_FAIL, "Delete IP Loopback Interface fail !"},
    /* --------------------loopback interface end------------------- */
    /* --------------------limit multicast range start------------------- */
    {WEB_ERROR_MCAST_DEL_ATTA_PORT, "Please delete attached port first."},
    /* --------------------limit multicast range end------------------- */
    /* --------------------delete firmware start------------------- */
    {WEB_ERROR_DEL_FIRM_LAST_CANNOT_DEL,"The last firmware cannot be deleted."},
    {WEB_ERROR_DEL_FIRM_INVALID_IMAGE,"Invalid image id!"},
    {WEB_ERROR_DEL_FIRM_CANNOT_DEL_BOOT,"Error : Can't delete Boot firmware !"},
    /* --------------------delete firmware end------------------- */
    /* --------------------TFTP message start--------------------*/
    {WEB_ERROR_TFTP_WRITTINNG, "The TFTP process is writting, it can not stop."},
    {WEB_ERROR_TFTP_NOT_EXIST, "The TFTP process is not exit."},
    /* --------------------TFTP message end--------------------*/
    /* --------------------check trunk message start--------------------*/
	{WEB_ERROR_TRAFFIC_CONTROL_CHECK_TRUNK_FAIL, "Truck Member portlist cannot be set when packet storm control is enabled!"},
	{WEB_ERROR_LBD_CHECK_TRUNK_FAIL, "Loopdetect port setting is difference between master port and member ports!"},
	{WEB_ERROR_BANDWIDTH_CONTROL_CHECK_TRUNK_FAIL, "Bandwidth_control setting is difference between master port and member ports!"},
	{WEB_ERROR_8021Q_CHECK_TRUNK_FAIL, "802.1p port setting is difference between master port and member ports!"},
	{WEB_ERROR_MIRROR_OVERLAP_CHECK_TRUNK_FAIL, "Trunk Member portlist cannot overlap the Mirror Target port!"},
	{WEB_ERROR_MIRROR_CHECK_TRUNK_FAIL, "Mirror port setting is difference between master port and member ports."},
	{WEB_ERROR_TRAFFIC_SEG_CHECK_TRUNK_FAIL, "Traffic segmentation port setting is difference between master port and member ports!"},
	{WEB_ERROR_PORT_CHECK_TRUNK_FAIL, "Port setting is difference between master port and member ports!"},
	{WEB_ERROR_SEC_PORT_OVERLAP_CHECK_TRUNK_FAIL, "Trunk Member portlist cannot overlap the Port-Security port!"},
	{WEB_ERROR_VLAN_CHECK_TRUNK_FAIL, "VLAN port setting is difference between master port and member portst!"},
	{WEB_ERROR_STP_CHECK_TRUNK_FAIL, "STP port setting is difference between master port and member ports!"},
	{WEB_ERROR_ADDRESS_BINDING_CHECK_TRUNK_FAIL, "Trunk Member portlist cannot overlap the Address_Binding port!"},
	{WEB_ERROR_MULTICAST_FILTER_CHECK_TRUNK_FAIL, "Multicast filter port setting is difference between master port and member ports!"},
	{WEB_ERROR_DHCP_FILTER_CHECK_TRUNK_FAIL, "DHCP filter port setting is difference between master port and member ports."},
	{WEB_ERROR_LACP_CHECK_VLAN_FAIL,"LACP port can not be in different vlan setting."},
    {WEB_ERROR_8021V_CHECK_TRUNK_FAIL, "Trunk Member portlist cannot overlap the 802.1v port!"},
    {WEB_ERROR_MBA_CHECK_TRUNK_FAIL, "Trunk Member portlist cannot overlap the MBA ports!"},
    /* --------------------check trunk message end--------------------*/
    /* --------------------check port security message start--------------------*/
    {WEB_ERROR_CHECK_IPB_PORTLIST_FAIL, "IP-MAC Binding port state is enabled on some ports!"},
    /* --------------------check port security message end--------------------*/
	/* --------------------DHCP server message start--------------------*/
    {WEB_ERROR_DHCP_INIT_FAIL, "Module not initialized or fail to initialize!"},
    {WEB_ERROR_DHCP_ENTRY_ALREADY_EXIST, "Entry exists!"},
    {WEB_ERROR_DHCP_ENTRY_NOT_FOUND, "Entry not found!"},
    {WEB_ERROR_DHCP_TABLE_FULL, "Table full!"},
    {WEB_ERROR_DHCP_INVALID_INPUT, "Invalid input parameter(s)!"},
    {WEB_ERROR_DHCP_INVALID_POOL_NAME, "Invalid pool name!"},
    {WEB_ERROR_DHCP_NETWORK_OVERLAP, "Network overlaps!"},
    {WEB_ERROR_DHCP_POOL_NOT_FOUND, "Pool not found!"},
    {WEB_ERROR_DHCP_INVALID_PING_PACKET, "Invalid ping packet!"},
    {WEB_ERROR_DHCP_INVALID_PING_TIMEOUT, "Invalid ping timeout!"},
    {WEB_ERROR_DHCP_INVALID_EXCLUDE_ADDR, "Invalid excluded addresses!"},
    {WEB_ERROR_DHCP_INVALID_NETWORK, "Invalid network!"},
    {WEB_ERROR_DHCP_INVALID_DOMAIN_NAME, "Invalid domain name!"},
    {WEB_ERROR_DHCP_INVALID_NETBIOS_NODE_TYPE, "Invalid NetBIOS node type!"},
    {WEB_ERROR_DHCP_INVALID_NEXT_SERVER, "Invalid next server!"},
    {WEB_ERROR_DHCP_INVALID_BOOT_FILE, "Invalid boot file!"},
    {WEB_ERROR_DHCP_INVALID_LEASE_TIME, "Invalid lease time!"},
    {WEB_ERROR_DHCP_INVALID_DEFAULT_ROUTER, "Invalid default router!"},
    {WEB_ERROR_DHCP_INVALID_DNS_SERVER, "Invalid DNS server!"},
    {WEB_ERROR_DHCP_INVALID_NETBIOS_NAME_SERVER, "Invalid NetBIOS name server!"},
    {WEB_ERROR_DHCP_INVALID_MANUAL_BINDING_IP, "Invalid IP in manual binding!"},
    {WEB_ERROR_DHCP_INVALID_MANUAL_BINDING_HARDWARE_ADDR, "Invalid hardware address in manual binding!"},
    {WEB_ERROR_DHCP_DUPLICATE_MANUAL_BINDING_IP, "Duplicate IP in manual binding!"},
    {WEB_ERROR_DHCP_DUPLICATE_MANUAL_BINDING_HARDWARE_ADDR, "Duplicate hardware address in manual binding!"},
    {WEB_ERROR_DHCP_DHCP_RELAY_IS_ENABLED, "DHCP server can not be enabled since DHCP relay has been enabled!"},
    {WEB_ERROR_DHCP_DHCP_SERVER_IS_ENABLED, "DHCP relay can not be enabled since DHCP server has been enabled!"},
    {WEB_ERROR_DHCP_UNKNOW_ERR, "Unknown error!"},
	/* --------------------DHCP server message end-------------------- */
    /* --------------------IPIF message start-------------------- */
    {WEB_ERROR_IPIF_SEC_EXIST, "Secondary IP interface exists in the vlan!"},
    /* --------------------IPIF message end-------------------- */
    /* --------------------IMP message start-------------------- */
    {WEB_ERROR_IMP_NOT_ENOUGHT_PROFILES, "Warning! The switch does not have enough access profiles."},
    {WEB_ERROR_IMP_NOT_ENOUGHT_ENTRIES, "Warning! The switch does not have enough access entries."},
    /* --------------------IMP message end-------------------- */
	{0xffff , ""}
};
// Added by Vic Yu 2002-09/18

//==================  Slot Name String  ==================================
char *Web_UnitNameStr[]={"System",
						"1",
						"2",
						"3",
						"4",
						"5",
						"6",
						"7",
						"8",
						"9",
						"10",
						"11",
						"12",
						"13"};         // Added by Vic Yu 2002-10/18

char *html_SetBuffer(char *data)
{
	if (data)
        strcpy(szHtmlBuffer, data);
    else
    	memset(szHtmlBuffer,0,sizeof(szHtmlBuffer));
        //*szHtmlBuffer = 0;

    return szHtmlBuffer;
}

char *html_AppendBufferFormat(char *format, ...)
{
    va_list vlist;
    char    *sp;
	int szLen=strlen(szHtmlBuffer);

	if(szLen>HTML_BUFFER_SIZE) // We must limit the length under "HTML_BUFFER_SIZE=1280".
	{
		szLen=HTML_BUFFER_SIZE;
		szHtmlBuffer[szLen]=0;
	}
    sp = &szHtmlBuffer [szLen];

    va_start(vlist, format);
    vsprintf(sp, format, vlist);
    va_end  (vlist);

    return szHtmlBuffer;
}

char* hprintf(char *format, ...)
{
	int nUsed;
	va_list vlist;

	if(NULL == format)
	{
		nBuffUsed = 0;
		pszHtmlBufferHead = &szHtmlBuffer[0];
		pszHtmlBufferHead[0] = '\0';
		return pszHtmlBufferHead;
	}

	va_start(vlist, format);
	nUsed = vsprintf(pszHtmlBufferHead, format, vlist);
	va_end  (vlist);

	nBuffUsed += nUsed;
	if(nBuffUsed>HTML_BUFFER_SIZE) // We must limit the length under "HTML_BUFFER_SIZE=1280".
	{
		nBuffUsed=HTML_BUFFER_SIZE;
		szHtmlBuffer[nBuffUsed]=0;
	}
	pszHtmlBufferHead = &szHtmlBuffer[nBuffUsed];
	return szHtmlBuffer;
}

char* hprintf_Menu(char *format, ...)
{
	int nUsed;
	va_list vlist;

	if(NULL == format)
	{
		nBuffUsed_Menu = 0;
		pszHtmlBufferHead_Menu = &szHtmlBuffer_Menu[0];
		pszHtmlBufferHead_Menu[0] = '\0';
		return pszHtmlBufferHead_Menu;
	}

	va_start(vlist, format);
	nUsed = vsprintf(pszHtmlBufferHead_Menu, format, vlist);
	va_end  (vlist);

	nBuffUsed_Menu += nUsed;
	if(nBuffUsed_Menu>HTML_BUFFER_SIZE) // We must limit the length under "HTML_BUFFER_SIZE=1280".
	{
		nBuffUsed_Menu=HTML_BUFFER_SIZE;
		szHtmlBuffer_Menu[nBuffUsed_Menu]=0;
	}
	pszHtmlBufferHead_Menu = &szHtmlBuffer_Menu[nBuffUsed_Menu];
	return szHtmlBuffer_Menu;
}


int ErrorMegApplyFlag=0;
WEB_ERROR_VALUE ErrorValue=0;

char *ErrorMegMapping(WEB_ERROR_VALUE errorvalue)
{
	int i=0;

	while(ErrMegTable[i].IndexCode>=0) {
		if(ErrMegTable[i].IndexCode==errorvalue)
		{
			return ErrMegTable[i].ErrorMeg;
		}
		i++;
	}
	return "";
}


char *html_ErrorMessage_Get(void)
{
	char *Meg;

	HPF(NULL);
	if(ErrorMegApplyFlag && ErrorValue!=0)
	{
		ErrorMegApplyFlag=0;
		Meg=ErrorMegMapping(ErrorValue);
		ErrorValue=WEB_ERROR_NO_ERROR;
	   	HPF("<script>\n");
		HPF("alert(\"%s\");\n",Meg);
		HPF("</script>\n");
		return HPF("\n");
	}
	else
	{
		ErrorMegApplyFlag=0;
		return HPF("\n");
	}
}

void sWeb_DisplayString_Reword(char* pString, char* pResult)
{
	int i, j, iLen;

	iLen= strlen(pString);
	for( i=j=0; i< iLen; i++)
	{
#if 1
		if(pString[i]=='<')
		{
			pResult[j++]= '&';
			pResult[j++]= 'l';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='>')
		{
			pResult[j++]= '&';
			pResult[j++]= 'g';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='&')
		{
			pResult[j++]= '&';
			pResult[j++]= 'a';
			pResult[j++]= 'm';
			pResult[j++]= 'p';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='"')
		{
			pResult[j++]= '&';
			pResult[j++]= 'q';
			pResult[j++]= 'u';
			pResult[j++]= 'o';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
#endif
		pResult[j++]= pString[i];
	}
	pResult[j++]= '\0';
}
