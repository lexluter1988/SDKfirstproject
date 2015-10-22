// Initializes the SDK library and 
// logs to Parallels Cloud Server locally
// Obtains a handle of type PHT_SERVER identifying
// the Parallels Cloud Server
#include "SdkWrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PRL_RESULT LoginLocal(PRL_HANDLE &hServer)
{
	// Variables for handles.
	PRL_HANDLE hJob = PRL_INVALID_HANDLE; // job handle
	PRL_HANDLE hJobResult = PRL_INVALID_HANDLE; // job result

	// Variables for return codes.
	PRL_RESULT err = PRL_ERR_UNINITIALIZED;
	PRL_RESULT nJobReturnCode = PRL_ERR_UNINITIALIZED;

	// Use the correct dynamic library depending on the platform
	#define SDK_LIB_NAME "libprl_sdk.so"

	// Load SDK library.
	if (PRL_FAILED(SdkWrap_Load(SDK_LIB_NAME)) &&
		PRL_FAILED(SdkWrap_Load("./" SDK_LIB_NAME)))
	{
		fprintf( stderr, "Failed to load " SDK_LIB_NAME "\n");
		return -1;
	}
	else {
		printf( "Successfully loaded libprl_sdk.so\n");
	}
	// Initialize the API.
	err = PrlApi_InitEx(PARALLELS_API_VER, PAM_SERVER, 0, 0);

	if (PRL_FAILED(err))
	{
		fprintf(stderr, "PrlApi_InitEx returned with error: %s.\n",
				prl_result_to_string(err));
		PrlApi_Deinit();
		SdkWrap_Unload();
		return -1;
	}
    else {                                                                                                      
        printf( "Successfully loaded PrlAPI\n");                                            
	}
	
	// Create a server handle (PHT_SERVER).
	err = PrlSrv_Create(&hServer);
	if (PRL_FAILED(err))
	{
		fprintf(stderr, "PrlSrv_Create failed, error: %s",
				prl_result_to_string(err));
		PrlApi_Deinit();
		SdkWrap_Unload();
		return -1;
	}
	else {
		printf("Successfully created PrlSrv_Create HANDLE\n");
	}
	// Login in (asynchronous call).
	hJob = PrlSrv_LoginLocal(hServer, NULL, NULL, PSL_NORMAL_SECURITY);

	// Wait for a maximum of 10 for 
	// the job to complete.
	err = PrlJob_Wait(hJob, 1000);
	if (PRL_FAILED(err))
	{
		fprintf(stderr, "PrlJob_Wait for PrlSrv_LoginLocal returned with error: %s\n",
				prl_result_to_string(err));
		PrlHandle_Free(hJob);
		PrlHandle_Free(hServer);
		PrlApi_Deinit();
		SdkWrap_Unload();
		return -1;
	}
	else {
		printf("Successfully completed PrlSrv_LoginLocal job\n");
	}
	// getting networks list 
	hJob = PrlSrv_GetIPPrivateNetworksList(hServer, 0);
	err = PrlJob_Wait(hJob, 10000);
	if (PRL_FAILED(err))
	{
		fprintf(stderr, "PrlJob_Wait for PrlSrv_GetIPPrivateNetworksList returned with error: %s\n",
				prl_result_to_string(err));
		PrlHandle_Free(hJob);
		PrlHandle_Free(hServer);
		PrlApi_Deinit();
		SdkWrap_Unload();
		return -1;
	}
	else {
		printf("Successfully completed PrlSrv_GetIPPrivateNetworksList job\n");
	}
	PRL_UINT32 count, netscount, i;
	PRL_HANDLE vlan_list, obj, ip_list;
	PRL_RESULT rc;
	PRL_HANDLE_PTR list;
	char buf[1024] = "";
	PRL_UINT32 size = 1024;
	PRL_UINT32_PTR pnItemBufSize = &size;

	PRL_HANDLE_TYPE type = PHT_ERROR;;
	
	PRL_CONST_STR strhandle = "";

	rc  = PrlJob_GetResult(hJob, &vlan_list);
	if (PRL_FAILED(rc)) {
		printf("failed to get vlan_list\n"); 
	}
	else
		PrlHandle_GetType(vlan_list,&type);
		PrlDbg_HandleTypeToString(type, &strhandle);
		printf("Successfully to get vlan_list, handle type of vlan_list: %s\n", strhandle);

	rc = PrlResult_GetParamsCount(vlan_list, &count);
	if (PRL_FAILED(rc)) { 
		printf("failed to get count\n");
	}
	else
		printf("Successfully to get count %d\n", count);

	rc = PrlResult_GetParamByIndex(vlan_list, 0, &obj);
	if (PRL_FAILED(rc)) {
		printf("failed to get by index\n");
	}
	else 
		PrlHandle_GetType(obj,&type);
		PrlDbg_HandleTypeToString(type, &strhandle);
		printf("Successfully to get privnet handle, handle type of obj: %s\n", strhandle); 

	rc = PrlIPPrivNet_GetNetAddresses(obj, &ip_list);
	if (PRL_FAILED(rc)) {
		printf("failed to get ip addresses\n");	
	}
	else
		PrlHandle_GetType(ip_list,&type);
		PrlDbg_HandleTypeToString(type, &strhandle);
		printf("Successfully to get ip addresses list, handle type of ip_list: %s\n", strhandle);

	rc = PrlStrList_GetItemsCount(ip_list, &netscount);
	if (PRL_FAILED(rc)) {
		printf("failed to get ip count\n");
	}
	else 
		printf("Successfully to get ip count %d\n", netscount);

	printf("size of buffer %d\n", sizeof(buf));

	for(i = 0; i < netscount; i++) {
		rc = PrlStrList_GetItem(ip_list, i, buf, pnItemBufSize);
		if (PRL_FAILED(rc)) {
			printf("failed to get item from stringlist\n");
		}
		else
			printf("Successfully to get string of address list %s\n", buf);
	}

}

PRL_RESULT main()
{	
	PRL_HANDLE hServer = PRL_INVALID_HANDLE;
	LoginLocal(hServer);
}
