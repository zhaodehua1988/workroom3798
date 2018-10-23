#include "tsk_scene.h"
#include "tsk_player.h"
#include "tsk_go.h"
#include "sys_env.h"
#include "tsk_conf.h"
#include "tsk_fpga.h"
#include "sys_info.h"

#define TSK_SCENE_OUTPUT_ANGLE_FILE "./env/angle.txt"
#define TSK_SCENE_SETALPHA_SLEEP 35000
typedef struct TSK_SCENE_DEV_E
{
    WV_U32 sceneOpen;
    WV_U32  id;
    TSK_SCENE_INFO_S   scene;
    WV_U32 DevCascading;  // 级联信息 0: not sync;1:master 2: slave
    WV_U32 PlayMode;    //0:player循环播放 1:player随机播放
    WV_U32 volume[3];
    WV_U32 LightEna;    //0:disable 1:enable
    WV_U32 standby;    //0:standby disable (starting Up); 1:standby ena
    WV_U8  addOutline; //0:disable 1:enabale
    WV_U8  addAni;    //0:disable 1:enable
} TSK_SCENE_DEV_E;   
TSK_SCENE_DEV_E   gCurScene;

struct timeval SceneChangeTimeStart;
WV_U32 SceneChangeDataType;

/*******************************************************************
WV_S32 TSK_SCENE_GetLightEna();
*******************************************************************/
WV_S32 TSK_SCENE_GetLightEna()
{

    return gCurScene.LightEna;
}

/*******************************************************************
WV_S32 TSK_SCENE_SetLightEna(WV_U32 ena);
*******************************************************************/
WV_S32 TSK_SCENE_SetLightEna(WV_U32 ena)
{
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    if(ena ==0 || ena ==1 )
    {
        //printf("******set light ena[%d]\n",ena);
        gCurScene.LightEna=ena;
        sprintf(name, "LightEna");
        data = ena;
        SYS_ENV_SetU32(name,data);
        SYS_ENV_Save();
        return WV_SOK;
    }

    return WV_EFAIL;
}

/*******************************************************************
WV_S32 TSK_SCENE_GetPlayMode();
*******************************************************************/
WV_S32 TSK_SCENE_GetPlayMode()
{
    return gCurScene.PlayMode;
} 
/*******************************************************************
WV_S32 TSK_SCENE_ChangePlayMode(WV_U32 mode);
*******************************************************************/
WV_S32 TSK_SCENE_ChangePlayMode(WV_U32 mode)
{
    if(mode ==0 || mode ==1 )
    {
        //printf("goto +++++++  TSK_SCENE_ChangePlayMode\n");
        gCurScene.PlayMode=mode;
    }
    return WV_SOK;
}  
/*******************************************************************
WV_S32 TSK_SCENE_SetPlayMode(WV_U32 mode);
*******************************************************************/
WV_S32 TSK_SCENE_SetPlayMode(WV_U32 mode)
{
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    if(mode ==0 || mode ==1 )
    {
        gCurScene.PlayMode=mode;
        sprintf(name, "PlayMode");
        data = mode;
        SYS_ENV_SetU32(name,data);
        SYS_ENV_Save();
        return WV_SOK;
    }

    return WV_EFAIL;
} 


/*******************************************************************
WV_S32 TSK_SCENE_GetSyncEna();
*******************************************************************/
WV_S32 TSK_SCENE_GetSyncEna()
{
    return gCurScene.DevCascading;
}
/*******************************************************************

WV_S32 TSK_SCENE_SetCascading(WV_U8 cascadeInfo,WV_U16 cascadeNum,WV_U8 *pSlaveIp);

*******************************************************************/
WV_S32 TSK_SCENE_SetCascading(WV_U8 cascadeInfo,WV_U16 cascadeNum,WV_U8 *pSlaveIp)
{
    //printf("cascadeInfo = %d ,cascadeNum=%d \n",cascadeInfo,cascadeNum);

    if(cascadeInfo != TSK_SCENE_SYNC_NONE && cascadeInfo != TSK_SCENE_SYNC_MASTER && cascadeInfo != TSK_SCENE_SYNC_SLAVE)
    {
        return WV_EFAIL;
    }

    WV_S32 i;
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN],value[WV_CONF_VALUE_MAX_LEN];
    sprintf(name, "DevCascading");
    data =cascadeInfo;
    SYS_ENV_SetU32(name,data);
    //
    gCurScene.DevCascading = cascadeInfo;
    sprintf(name, "cascadeNum");
    data = cascadeNum;
    SYS_ENV_SetU32(name,data);

    for(i=0;i<cascadeNum;i++)
    {
        sprintf(name, "SlaveIp%d",i);
        sprintf(value,"%d.%d.%d.%d",pSlaveIp[4*i],pSlaveIp[4*i+1],pSlaveIp[4*i+2],pSlaveIp[4*i+3]);
        SYS_ENV_Set(name,value);
    }

    SYS_ENV_Save();
    return WV_SOK;

}
/*******************************************************************

WV_S32 TSK_SCENE_GetCascading(WV_U8 cascadeInfo,WV_U16 cascadeNum,WV_U8 *pSlaveIp);

*******************************************************************/
WV_S32 TSK_SCENE_GetCascading(WV_U8 *pCascadeInfo,WV_U16 *pCascadeNum,WV_U8 *pSlaveIp)
{	
    WV_S32 i;
    WV_U32 data,num;
    WV_S8  name[WV_CONF_NAME_MAX_LEN],value[WV_CONF_VALUE_MAX_LEN];

    i = 0;
    sprintf(name, "DevCascading",i);
    SYS_ENV_GetU32(name,&data);
    *pCascadeInfo = data;

    sprintf(name, "cascadeNum",i);
    SYS_ENV_GetU32(name,&data);
    *pCascadeNum = data;

    for(i=0;i<*pCascadeNum;i++)
    {

        sprintf(name, "SlaveIp%d",i);
        SYS_IP_getIpInt(name,&pSlaveIp[4*i]);
        //printf("get slave ip[%d]:%d.%d.%d.%d\n",i,pSlaveIp[4*i],pSlaveIp[4*i+1],pSlaveIp[4*i+2],pSlaveIp[4*i+3]);
    }

    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_RegisterConf();
*******************************************************************/
WV_S32 TSK_SCENE_RegisterConf()
{

    WV_S32 i,j;
    WV_S8  name[WV_CONF_NAME_MAX_LEN],value[WV_CONF_VALUE_MAX_LEN];


    // sprintf(name,"SceneAllConfig");
    SYS_ENV_Register(0,"SceneAllConfig",NULL,NULL);
    //
    sprintf(name, "SceneCurId");
    sprintf(value, "%d",0);
    SYS_ENV_Register(1 ,name,value,"scene top id");

    //mov area
    for(i=0;i<TSK_SCENE_MOV_MAX_NUM;i++)
    {

        sprintf(name, "SceneMov%dX",i);
        if(i==0 || i== 3)
        {
            sprintf(value, "%d",0);
        }else{

            sprintf(value, "%d",1920);
        }
        SYS_ENV_Register(1 ,name,value,"scene movie position x");
        //
        sprintf(name, "SceneMov%dY",i);
        if(i==0 || i== 1)
        {
            sprintf(value, "%d",0);
        }else{

            sprintf(value, "%d",1080);
        }

        SYS_ENV_Register(1 ,name,value,"scene movie position y");
        //
        sprintf(name, "SceneMov%dW",i);
        sprintf(value, "%d",1919);
        SYS_ENV_Register(1 ,name,value,"scene movie widow width");
        //
        sprintf(name, "SceneMov%dH",i);
        sprintf(value, "%d",1079);
        SYS_ENV_Register(1 ,name,value,"scene movie widow height");
    }
    for(i=0;i<TSK_SCENE_MAX_NUM;i++)
    {
        //
        sprintf(name,"Scene%dConfig",i);
        SYS_ENV_Register(0,name,NULL,NULL);
        //
        if(i<16)
        {
            sprintf(name, "Scene%dEnable",i);
            sprintf(value, "%d",1);
            SYS_ENV_Register(1 ,name,value,"scene enable");
        }else{
            sprintf(name, "Scene%dEnable",i);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene enable");
        }
        sprintf(name, "Scene%dType",i);
        sprintf(value, "%d",0);
        SYS_ENV_Register(1 ,name,value,"scene type");
        for(j=0;j<TSK_SCENE_MOV_MAX_NUM;j++)
        {
            sprintf(name, "Scene%dMov%dId",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene movie id");

            sprintf(name, "Scene%dMov%dEna",i,j);
            if(j==0)
            {
                sprintf(value, "%d",1);
            }else{
                sprintf(value, "%d",0);
            }
            SYS_ENV_Register(1 ,name,value,"scene movie widow width");
        }
        //*****************
        sprintf(name, "Scene%dAniNum",i);
        sprintf(value, "%d",0);
        SYS_ENV_Register(1 ,name,value,"scene animation number");
        //
        for(j = 0;j<TSK_SCENE_GIF_MAX_NUM;j++)
        {
            sprintf(name, "Scene%dAni%dId",i,j);
            sprintf(value, "%d",j);
            SYS_ENV_Register(1 ,name,value,"scene animation file ID");
            //
            sprintf(name, "Scene%dAni%dX",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene animation x");
            //
            sprintf(name, "Scene%dAni%dY",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene animation y");
            //
            sprintf(name, "Scene%dAni%dW",i,j);
            sprintf(value, "%d",800);
            SYS_ENV_Register(1 ,name,value,"scene animation width");
            //
            sprintf(name, "Scene%dAni%dH",i,j);
            sprintf(value, "%d",600);
            SYS_ENV_Register(1 ,name,value,"scene animation height");
        }

        sprintf(name, "Scene%dwinNum",i);
        sprintf(value, "%d",0);
        SYS_ENV_Register(1 ,name,value,"scene window number");
        for(j = 0 ;j<TSK_SCENE_WIN_MAX_NUM;j++)
        {
            sprintf(name, "Scene%dWin%dOutId",i,j);
            sprintf(value, "%d",j);
            SYS_ENV_Register(1 ,name,value,"scene window Outline ID");
            //
            sprintf(name, "Scene%dWin%dX",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene window x");
            //
            sprintf(name, "Scene%dWin%dY",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene window y");
            //
            sprintf(name, "Scene%dWin%dW",i,j);
            sprintf(value, "%d",800);
            SYS_ENV_Register(1 ,name,value,"scene window width");
            //
            sprintf(name, "Scene%dWin%dH",i,j);
            sprintf(value, "%d",600);
            SYS_ENV_Register(1 ,name,value,"scene window height");
            //
            sprintf(name, "Scene%dWin%dVid",i,j);
            sprintf(value, "%d",0);
            SYS_ENV_Register(1 ,name,value,"scene win video id");
        }
    }

    sprintf(name, "DevCascading");
    sprintf(value, "%d",0);
    SYS_ENV_Register(1 ,name,value,"scene sync cmd");

    sprintf(name, "cascadeNum",i);
    sprintf(value, "%d",0);
    SYS_ENV_Register(1 ,name,value,"sync client num");//0:unEna ip;1:Ena

    for(i=0;i<TSK_SCENE_MAX_SYNCDEV;i++)
    {
        //gCurScene
        sprintf(name, "SlaveIp%d",i);
        sprintf(value, "192.168.15.9%d",i);
        SYS_ENV_Register(1 ,name,value,"sync client ip");

    }

    sprintf(name, "PlayMode");
    sprintf(value, "%d",0);
    SYS_ENV_Register(1 ,name,value,"scene sync cmd");

    sprintf(name, "LightEna");
    sprintf(value, "%d",1);
    SYS_ENV_Register(1 ,name,value,"light ena flag");


    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_GetScene(WV_U32  id ,TSK_SCENE_INFO_S  * pDev);
*******************************************************************/
WV_S32 TSK_SCENE_GetScene(WV_U32  id ,TSK_SCENE_INFO_S  * pDev)
{
    WV_S32 i,j;
    WV_U32 data,*pData;
    pData = & data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    sprintf(name, "Scene%dEnable",id);
    SYS_ENV_GetU32(name, & data);
    pDev->sceneEna = data;

    for(i=0; i<TSK_SCENE_MOV_MAX_NUM; i++)
    {
        sprintf(name, "Scene%dMov%dId",id,i);
        SYS_ENV_GetU32(name, & data);
        pDev->mov[i].id = data;

        sprintf(name, "Scene%dMov%dEna",id,i);
        SYS_ENV_GetU32(name, & data);
        pDev->mov[i].ena = data;
        //

        sprintf(name, "SceneMov%dX",i);
        SYS_ENV_GetU32(name,pData);
        pDev->mov[i].x = data;
        //
        sprintf(name, "SceneMov%dY",i);
        SYS_ENV_GetU32(name,pData);
        pDev->mov[i].y = data;
        //
        sprintf(name, "SceneMov%dW",i);
        SYS_ENV_GetU32(name,pData);
        pDev->mov[i].w = data;
        //
        sprintf(name, "SceneMov%dH",i);
        SYS_ENV_GetU32(name,pData);
        pDev->mov[i].h = data;

        //printf("i[%d]x%d,y%d,w%d,h%d\n",i,pDev->mov[i].x,pDev->mov[i].y,pDev->mov[i].w,pDev->mov[i].h);
    }
    //*****************************
    sprintf(name, "Scene%dAniNum",id);
    SYS_ENV_GetU32(name,pData);
    pDev->animationNum  = data;
    //
    for(j = 0;j<TSK_SCENE_GIF_MAX_NUM;j++)
    {
        sprintf(name, "Scene%dAni%dId",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->animation[j].id  = data;
        //
        sprintf(name, "Scene%dAni%dX",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->animation[j].x  = data;
        //
        sprintf(name, "Scene%dAni%dY",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->animation[j].y  = data;
        //
        sprintf(name, "Scene%dAni%dW",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->animation[j].w = data;
        //
        sprintf(name, "Scene%dAni%dH",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->animation[j].h  = data;

    }

    sprintf(name, "Scene%dwinNum",id);
    SYS_ENV_GetU32(name,pData);
    pDev->winNum  = data;

    for(j = 0 ;j<TSK_SCENE_WIN_MAX_NUM;j++)
    {
        sprintf(name, "Scene%dWin%dOutId",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].outId  = data;
        //
        sprintf(name, "Scene%dWin%dX",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].x  = data;
        //
        sprintf(name, "Scene%dWin%dY",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].y  = data;
        //
        sprintf(name, "Scene%dWin%dW",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].w  = data;
        //
        sprintf(name, "Scene%dWin%dH",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].h  = data;
        //
        sprintf(name, "Scene%dWin%dVid",id,j);
        SYS_ENV_GetU32(name,pData);
        pDev->win[j].videoId  = data;
    }

    //printf("get scene sync cmd regest\n");
    sprintf(name, "DevCascading");
    SYS_ENV_GetU32(name,pData);
    gCurScene.DevCascading  = data;

    sprintf(name, "PlayMode");
    SYS_ENV_GetU32(name,pData);
    gCurScene.PlayMode = data;

    sprintf(name, "LightEna");
    SYS_ENV_GetU32(name,pData);
    gCurScene.LightEna = data;

    return WV_SOK;

}

/*******************************************************************
 WV_S32 TSK_SCENE_SaveScene(WV_U32  id ,TSK_SCENE_INFO_S  * pDev);
*******************************************************************/
WV_S32 TSK_SCENE_SaveScene(WV_U32  id ,TSK_SCENE_INFO_S  * pDev)
{
    WV_S32 i, j;
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    pDev->sceneEna= 1; //set scene ena
    sprintf(name, "Scene%dEnable",id);
    data = pDev->sceneEna;
    SYS_ENV_SetU32(name,data);
    //
    for(i=0;i<TSK_SCENE_MOV_MAX_NUM;i++)
    {
        sprintf(name, "Scene%dMov%dId",id,i);
        data = pDev->mov[i].id;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dMov%dEna",id,i);
        data = pDev->mov[i].ena;
        SYS_ENV_SetU32(name,data);

        sprintf(name, "Scene%dMov%d",id,i);
        data = pDev->mov[i].id;
        SYS_ENV_SetU32(name,data);

        sprintf(name, "SceneMov%dX",i);
        data = pDev->mov[i].x;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "SceneMov%dY",i);
        data = pDev->mov[i].y;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "SceneMov%dW",i);
        data = pDev->mov[i].w;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "SceneMov%dH",i);
        data = pDev->mov[i].h;
        SYS_ENV_SetU32(name,data);
        //*****************
        sprintf(name, "Scene%dAniNum",id);
        data = pDev->animationNum;
        SYS_ENV_SetU32(name,data);
    }
    //
    for(j = 0;j<TSK_SCENE_GIF_MAX_NUM;j++)  //gif num +1 ,because one pic for usb or phone
    {
        sprintf(name, "Scene%dAni%dId",id,j);
        data = pDev->animation[j].id;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dAni%dX",id,j);
        data = pDev->animation[j].x;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dAni%dY",id,j);
        data = pDev->animation[j].y;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dAni%dW",id,j);
        data = pDev->animation[j].w;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dAni%dH",id,j);
        data = pDev->animation[j].h;
        SYS_ENV_SetU32(name,data);
    }

    sprintf(name, "Scene%dwinNum",id);
    data = pDev->winNum;
    SYS_ENV_SetU32(name,data);
    for(j = 0 ;j<TSK_SCENE_WIN_MAX_NUM;j++)
    {
        sprintf(name, "Scene%dWin%dOutId",id,j);
        data = pDev->win[j].outId;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dWin%dX",id,j);
        data = pDev->win[j].x;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dWin%dY",id,j);
        data = pDev->win[j].y;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dWin%dW",id,j);
        data = pDev->win[j].w;
        SYS_ENV_SetU32(name,data);
        //
        sprintf(name, "Scene%dWin%dH",id,j);
        data = pDev->win[j].h;
        SYS_ENV_SetU32(name,data);;
        //
        sprintf(name, "Scene%dWin%dVid",id,j);
        data = pDev->win[j].videoId;
        SYS_ENV_SetU32(name,data);
    }

    SYS_ENV_Save();
    return WV_SOK;

}
/*******************************************************************
 WV_S32 TSK_SCENE_Save(WV_U32  id );
*******************************************************************/

WV_S32 TSK_SCENE_Save(WV_U32  id)
{

    WV_ASSERT_RET ( TSK_SCENE_SaveScene(id ,&gCurScene.scene));
    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_SaveCustom(WV_U32  id );
*******************************************************************/

WV_S32 TSK_SCENE_SaveCustom(WV_U32  id,TSK_CONF_SCENE_CONF *pScene)
{
    WV_ASSERT_RET ( TSK_SCENE_SaveSceneCustom(id ,&gCurScene.scene,pScene));
    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_SaveScene(WV_U32  id ,TSK_SCENE_INFO_S  * pDev);
*******************************************************************/
WV_S32 TSK_SCENE_SaveSceneCustom(WV_U32  id ,TSK_SCENE_INFO_S  * pDev,TSK_CONF_SCENE_CONF *pScene)
{
    WV_S32 i, j;
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];

    pDev->sceneEna = 1; //set scene ena
    sprintf(name, "Scene%dEnable", id);
    data = pDev->sceneEna;
    SYS_ENV_SetU32(name, data);
    for (i = 0; i < TSK_SCENE_MOV_MAX_NUM; i++)
    {
        sprintf(name, "Scene%dMov%dId", id, i);
        if (TSK_CONF_Mov_GetIDByName(&data, (WV_S8*)&pScene->ucSceneMovName[i].ucVideoFileName[0]) == WV_SOK)
        {
            SYS_ENV_SetU32(name, data);
        }
        else
        {
            SYS_ENV_SetU32(name, 0);
        }
        sprintf(name, "Scene%dMov%dEna", id, i);
        data = pDev->mov[i].ena;
        SYS_ENV_SetU32(name, data);
        sprintf(name, "Scene%dMov%d", id, i);
        if (TSK_CONF_Mov_GetIDByName(&data, (WV_S8*)&pScene->ucSceneMovName[i].ucVideoFileName[0]) == WV_SOK)
        {
            SYS_ENV_SetU32(name, data);
        }
        else
        {
            SYS_ENV_SetU32(name, 0);
        }
        sprintf(name, "SceneMov%dX", i);
        data = pDev->mov[i].x;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "SceneMov%dY", i);
        data = pDev->mov[i].y;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "SceneMov%dW", i);
        data = pDev->mov[i].w;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "SceneMov%dH", i);
        data = pDev->mov[i].h;
        SYS_ENV_SetU32(name, data);
        //*****************
        sprintf(name, "Scene%dAniNum", id);
        data = pScene->ucBKFrameCount;
        SYS_ENV_SetU32(name, data);
    }
    //
    for (j = 0; j < TSK_SCENE_GIF_MAX_NUM; j++)  //gif num +1 ,because one pic for usb or phone
    {
        sprintf(name, "Scene%dAni%dId", id, j);
        data = pScene->BKFrame[j].usID;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dAni%dX", id, j);
        data = pScene->BKFrame[j].usLeft;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dAni%dY", id, j);
        data = pScene->BKFrame[j].usTop;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dAni%dW", id, j);
        data = pScene->BKFrame[j].usRight;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dAni%dH", id, j);
        data = pScene->BKFrame[j].usBottom;
        SYS_ENV_SetU32(name, data);
    }
    sprintf(name, "Scene%dwinNum", id);
    data = pScene->ucOutLineCount;
    SYS_ENV_SetU32(name, data);
    for (j = 0; j < TSK_SCENE_WIN_MAX_NUM; j++)
    {
        sprintf(name, "Scene%dWin%dOutId", id, j);
        data = pScene->OutLine[j].usID;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dWin%dX", id, j);
        data = pScene->OutLine[j].usLeft;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dWin%dY", id, j);
        data = pScene->OutLine[j].usTop;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dWin%dW", id, j);
        data = pScene->OutLine[j].usRight;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dWin%dH", id, j);
        data = pScene->OutLine[j].usBottom;
        SYS_ENV_SetU32(name, data);
        //
        sprintf(name, "Scene%dWin%dVid", id, j);
        data = pScene->OutLine[j].usChannel;
        SYS_ENV_SetU32(name, data);
    }
    SYS_ENV_Save();
    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_Delet(WV_U32  id );
*******************************************************************/

WV_S32 TSK_SCENE_Delet(WV_U32  id)
{
    //pDev->sceneEna= 1; //set scene ena
    WV_S32 i;
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    sprintf(name, "Scene%dEnable",id);
    data = 0;
    SYS_ENV_SetU32(name,data);

    if(id == gCurScene.id)
    {
        for(i=0;i<TSK_SCENE_MAX_NUM;i++)
        {
            sprintf(name, "Scene%dEnable",i);
            SYS_ENV_GetU32(name,&data);
            if(data==1)
            {
                TSK_SCENE_Change(TSK_SCENE_TYPE_NETDATA, i);
                break;
            }
        }

    }
    SYS_ENV_Save();
    return WV_SOK;
}


/*******************************************************************
 WV_S32 TSK_SCENE_SetDefault(WV_U32  id );
*******************************************************************/
WV_S32 TSK_SCENE_SetDefault(WV_U32  id )
{

    SYS_ENV_SetU32("SceneCurId",id);
    //SYS_ENV_Save();
    WV_ASSERT_RET ( TSK_SCENE_SaveScene(id ,&gCurScene.scene));
    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_GetConf();
*******************************************************************/

WV_S32 TSK_SCENE_GetConf()
{
    WV_S32 ret = 0;
    WV_U32 data,*pData;
    pData = & data;

    SYS_ENV_GetU32("SceneCurId",pData);
    gCurScene.id = data;
    TSK_SCENE_GetScene(gCurScene.id,&gCurScene.scene);
    return ret;

}

/*******************************************************************
 WV_S32 TSK_SCENE_GetSceneCurId();
*******************************************************************/
WV_S32 TSK_SCENE_GetSceneCurId()
{

    return gCurScene.id;

}
/*******************************************************************
WV_S32 TSK_SCENE_GetCurent(TSK_SCENE_INFO_S *pInfo);
*******************************************************************/

WV_S32 TSK_SCENE_GetCurent(TSK_SCENE_INFO_S *pInfo)
{

    memcpy(pInfo,&gCurScene.scene,sizeof(TSK_SCENE_INFO_S));

    return 0;
}
/*******************************************************************
 WV_S32 TSK_SCENE_SceneInit();
*******************************************************************/
WV_S32 TSK_SCENE_SceneInit()
{

    WV_S32 i,j=0;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    // for movi

    gCurScene.sceneOpen = 1;

    TSK_PLAYER_ChangeMode(TSK_PLAYER_MODE_3840_2160);
    for(i=0;i<TSK_SCENE_MOV_USE_NUM;i++)
    {

        sprintf(name, "./mov/mov%d.mp4",gCurScene.scene.mov[i].id);
        if(access(name,0)!=0){

            continue;
            for(j=0;j<128;j++)
            {
                sprintf(name,"./mov/mov%d.mp4",j);
                if(access(name,0) == 0)
                {
                    break;
                }
            }
        }

        //	if(j>127)
        //	{
        //		break;
        //	}


        if(gCurScene.scene.mov[i].ena == 1)
        {

            if(gCurScene.scene.mov[i].y==0 && gCurScene.scene.mov[i].x==0 && gCurScene.scene.mov[i].w==0 && gCurScene.scene.mov[i].h == 0 )
            {
                //gCurScene.scene.mov[i].ena = 0;
                break;
            }

            TSK_PLAYER_SetFile(i,name);
            TSK_PLAYER_setWin(i,gCurScene.scene.mov[i].x,gCurScene.scene.mov[i].y,gCurScene.scene.mov[i].w,gCurScene.scene.mov[i].h);
            TSK_PLAYER_Creat(i);
            //sleep(1);
            //TSK_PLAYER_DevCascade(i,gCurScene.DevCascading);
        }

    }
    // for animation
    sleep(1);

    TSK_FPGA_load();//load

    TSK_PLAYER_ChangeMode(TSK_PLAYER_MODE_1920_1080);

    usleep(300000);
    HIS_FB_ClrFpga();
    usleep(300000);
    HIS_FB_ClrFpga();

    TSK_SCENE_ConfAni();
    //for win

    WV_ASSERT_RET ( FPGA_CONF_SetWin (gCurScene.scene.winNum ,gCurScene.scene.win));
    //usleep(3000000);
    HIS_FB_ClrFpga();//
    //FPGA_CONF_RqMcu();
    FPGA_CONF_ClrBuf();
    //open player volume
    TSK_PLAYER_ReadVolume();
    return WV_SOK;

} 
/*******************************************************************
 WV_S32 TSK_SCENE_GetState();

scene is open:return 1;
scene is close:return 0;
*******************************************************************/
WV_S32 TSK_SCENE_GetState()
{
    return 	gCurScene.sceneOpen;
}
/*******************************************************************
 WV_S32 TSK_SCENE_SceneOpen();
*******************************************************************/

WV_S32 TSK_SCENE_SceneOpen()
{

    if(gCurScene.sceneOpen == 1 || gCurScene.standby == 1)
    {
        WV_printf("*********scene is open  or dev is standby*************** \n");
        return 0;
    }
    WV_printf("*********scene open *************** \n");
    WV_S32 i,j;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];

    TSK_PLAYER_ChangeMode(TSK_PLAYER_MODE_1920_1080);//virture screen	size 1920*1080
    //for player
    for(i=0;i<TSK_SCENE_MOV_USE_NUM;i++)
    {
        if(gCurScene.scene.mov[i].ena == 0)
            continue;
        sprintf(name, "./mov/mov%d.mp4",gCurScene.scene.mov[i].id);
        if(access(name,0)!=0){
            continue;
        }
        if(gCurScene.scene.mov[i].y==0 && gCurScene.scene.mov[i].x==0 && gCurScene.scene.mov[i].w==0 && gCurScene.scene.mov[i].h == 0 )
        {
            //gCurScene.scene.mov[i].ena = 0;
            break;
        }
        TSK_PLAYER_SetFile(i,name);
        TSK_PLAYER_setWin(i,gCurScene.scene.mov[i].x,gCurScene.scene.mov[i].y,gCurScene.scene.mov[i].w,gCurScene.scene.mov[i].h);
        TSK_PLAYER_Creat(i);


    }
    TSK_PLAYER_ReadVolume();
//    HIS_FB_ClrFpga();
    if((TSK_USB_GetShowFlag() == 0) && (TSK_Mobile_GetShowFlag()==0))
    {

        //for win
        FPGA_CONF_SetWin (gCurScene.scene.winNum ,gCurScene.scene.win);

        // for animation
        TSK_SCENE_ConfAni();
//        HIS_FB_ClrFpga();

    }

    if(gCurScene.DevCascading == 1)
    {
        usleep(1);
        TSK_SCENE_SendSync();
    }

    gCurScene.sceneOpen=1;

    return WV_SOK;

}


/*******************************************************************

        WV_S32 TSK_SCENE_SceneClose();

*******************************************************************/

WV_S32 TSK_SCENE_SceneClose()
{

    if(gCurScene.sceneOpen == 0 || gCurScene.standby == 1)
    {
        WV_printf("scene is close  or dev is standby\n");
        return 0;
    }

    WV_S32 ret = 0,i;

    if((TSK_USB_GetShowFlag() == 0) && (TSK_Mobile_GetShowFlag()==0))
    {

        // for animation
        for(i = 0;i< HIS_HIGO_GODEC_NUM ; i++ )
        {
            TSK_GO_DecClose(i);
        }
//        sleep(1);
        //gCurScene.scene.winNum = 0;
        FPGA_CONF_SetWin (0 ,gCurScene.scene.win);
        // for movi
    }
    for(i=0;i<TSK_SCENE_MOV_USE_NUM-1;i++)
    {

        TSK_PLAYER_Destory(i);
    }

    gCurScene.sceneOpen = 0;
    WV_printf("\n****scene close ***********\n");

    return ret;

}
/*******************************************************************
 WV_S32 TSK_SCENE_Standby();
设备待机
*******************************************************************/
WV_S32 TSK_SCENE_Standby() 
{
    if(gCurScene.standby  == 1 )
    {
        return WV_SOK;
    }
    gCurScene.standby = 1;

    WV_S32 ret = 0,i;
    // for animation
    for(i = 0;i< HIS_HIGO_GODEC_NUM ; i++ )
    {
        TSK_GO_DecClose(i);
    }
    sleep(1);
    //gCurScene.scene.winNum = 0;
    FPGA_CONF_SetWin (0 ,gCurScene.scene.win);
    // for movi
    for(i=0;i<TSK_SCENE_MOV_USE_NUM-1;i++)
    {

        TSK_PLAYER_Destory(i);
    }

    TSK_PLAYER_FreezeWindowBlack();
    gCurScene.standby = 1;
    return WV_SOK;
}
/*******************************************************************
 WV_S32 TSK_SCENE_StartingUP();
设备启动
*******************************************************************/
WV_S32 TSK_SCENE_StartingUP() 
{

    if(gCurScene.standby  == 0 )
    {
        return WV_SOK;
    }

    /*
        TSK_SCENE_SceneInit();
        if(SVR_CONTROL_GetKtvDev() == 0)//雷石点歌机，初始化时默认歌曲随机播放
        {
                TSK_SCENE_ChangePlayMode(1);
        }
        */

    SYS_INFO_DevReset();
    gCurScene.standby = 0;
    return WV_SOK;
}

/*******************************************************************
WV_S32 TSK_SCENE_Sync();
*******************************************************************/
WV_S32 TSK_SCENE_Sync()
{
    WV_S32 ret=0;
    pthread_detach(pthread_self());
    ret = SVR_UDP_PlayerSync();
    return ret;
} 

/*******************************************************************
 WV_S32 TSK_SCENE_SendSync();
*******************************************************************/
WV_S32 TSK_SCENE_SendSync()
{


    if(gCurScene.DevCascading != 1)
    {
        return WV_SOK;
    }
    WV_THR_HNDL_T thrHndl;
    WV_CHECK_RET( WV_THR_Create(&thrHndl,TSK_SCENE_Sync, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, NULL));
    return WV_SOK;
}

WV_S32 TSK_SCENE_SetAlpha()
{
    WV_S32 i;

    for (i = 250; i > 10;)
    {
        HIS_FB_SetAlpha(i);
        usleep(TSK_SCENE_SETALPHA_SLEEP);
        if (i > 150)
        {
            i -= 25;
        }
        else
        {
            i -= 5;
        }
    }
    HIS_FB_clrDisPlayBmp();
    HIS_FB_SetAlpha(255);
    return WV_SOK;
}

WV_S32 TSK_SCENE_GetChangeTimeOut()
{
    float TimeUse = 0;
    struct timeval SceneChangeTimeEnd;

    gettimeofday(&SceneChangeTimeEnd, NULL);
    TimeUse = (SceneChangeTimeEnd.tv_sec - SceneChangeTimeStart.tv_sec) * 1000000 + (SceneChangeTimeEnd.tv_usec - SceneChangeTimeStart.tv_usec);
    if (TimeUse < 10000000.0)
    {
        return WV_EFAIL;
    }
    else
    {
        return WV_SOK;
    }
}

/*******************************************************************
 WV_S32 TSK_SCENE_Change( WV_U32  id);
*******************************************************************/
WV_S32 TSK_SCENE_Change(WV_U32 DataType, WV_U32  id)
{
    //printf("change scene to [%d]\n",id);
    if ((DataType == TSK_SCENE_TYPE_UARTDATA) && (SceneChangeDataType == TSK_SCENE_TYPE_NETDATA))
    {
        if (TSK_SCENE_GetChangeTimeOut() != WV_SOK)
        {
            return WV_SOK;
        }
    }
    SceneChangeDataType = DataType;
    gettimeofday(&SceneChangeTimeStart, NULL);

    if(gCurScene.DevCascading == 1 )
    {
        SVR_UDP_SyncScene(id);
    }
    gCurScene.id = id;
    WV_CHECK_RET(TSK_SCENE_GetScene(gCurScene.id,&gCurScene.scene));
    WV_S32 i;
    for(i=0;i<TSK_SCENE_MOV_USE_NUM;i++)
    {
        if(gCurScene.scene.mov[i].ena == 0)
            continue;
        TSK_PLAYER_setWin(i,gCurScene.scene.mov[i].x,gCurScene.scene.mov[i].y,gCurScene.scene.mov[i].w,gCurScene.scene.mov[i].h);
    }
    if(gCurScene.scene.sceneEna == 1)
    {
        WV_S32 Ret = WV_EFAIL;
        WV_U8 Fade = 0;
        if (TSK_CONF_LoadFadeinout(&Fade) == WV_SOK)
        {
            if (Fade == 1)
            {// 软件
                WV_CHECK_RET(TSK_SCENE_SceneClose());
                Ret = TSK_PLAYER_VidFrameToPicture(0);
                if ((gCurScene.scene.mov[1].w > 0) && (gCurScene.scene.mov[1].h > 0))
                {
                    Ret |= TSK_PLAYER_VidFrameToPicture(1);
                }
                WV_CHECK_RET(TSK_SCENE_SceneOpen());
                if (Ret == WV_SOK)
                {
                    TSK_SCENE_SetAlpha();
                }
            }
            else
            {
                WV_CHECK_RET(TSK_SCENE_SceneClose());
                WV_CHECK_RET(TSK_SCENE_SceneOpen());
            }
        }
        else
        {
            WV_CHECK_RET(TSK_SCENE_SceneClose());
            WV_CHECK_RET(TSK_SCENE_SceneOpen());
        }
        printf("TSK_SCENE_Change--------------------------\n");
        TSK_SCENE_SetDefault(id);
    }
    else
    {
        return WV_EFAIL;
    }
    return WV_SOK;
} 

/*******************************************************************
 WV_S32 TSK_SCENE_Restart();
*******************************************************************/

WV_S32 TSK_SCENE_Restart()
{
    WV_CHECK_RET( TSK_SCENE_SceneClose());
    WV_CHECK_RET(  TSK_SCENE_SceneOpen());
    return WV_SOK;

}

/*******************************************************************
 WV_S32 TSK_SCENE_PlayMovUrl(WV_U8 cmd,WV_U8 *url,WV_S32 len);
*******************************************************************/
WV_S32 TSK_SCENE_PlayMovUrl(WV_U8 cmd,WV_U8 *url,WV_S32 len)
{
    WV_printf("url:%s\n",url);

    WV_U8 name[64];
    memset(name,0,sizeof(name));
    sprintf(name,"%s",url);
    TSK_GO_MOV_PIC_POS_S movPos,picPos;
    TSK_GO_GetUsb_PicAndMov_Pos(&movPos,&picPos);

    if(cmd == 3)//stop the player
    {
        TSK_PLAYER_setWin(2,0,0,0,0);
        TSK_PLAYER_Destory(2);
        return WV_SOK;
    }

    if(movPos.startX == 0 && movPos.startY == 0 && (movPos.endX-movPos.startX)<200 && (movPos.endY-movPos.startY)<200)
    {
        return WV_EFAIL;
    }

    TSK_PLAYER_Destory(2);
    TSK_PLAYER_SetFile(2,url);
    TSK_PLAYER_setWin(2,movPos.startX,movPos.startY,movPos.endX-movPos.startX,movPos.endY-movPos.startY);
    TSK_PLAYER_Creat(2);
    TSK_PLAYER_ReadVolume();
    return WV_SOK;
} 

/*******************************************************************
 WV_S32 TSK_SCENE_SetMov(WV_U32 id);
*******************************************************************/
WV_S32 TSK_SCENE_SetMov(WV_U32 num,WV_U32 id)
{
    gCurScene.scene.mov[num].id = id;
    return WV_SOK;
}
/*******************************************************************

 WV_S32 TSK_SCENE_SetArea(WV_U32 id);

*******************************************************************/
WV_S32 TSK_SCENE_SetMovArea(WV_U32 num,WV_U32 ena,WV_U32 x,WV_U32 y,WV_U32 w,WV_U32 h)
{
    WV_U32 data;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];

    gCurScene.scene.mov[num].ena = ena;
    gCurScene.scene.mov[num].x= x;
    gCurScene.scene.mov[num].y= y;
    gCurScene.scene.mov[num].w= w;
    gCurScene.scene.mov[num].h= h;

    sprintf(name,"Scene%dMov%dEna",gCurScene.id,num);
    data = gCurScene.scene.mov[num].ena;
    SYS_ENV_SetU32(name,data);

    sprintf(name, "SceneMov%dX",num);
    data = gCurScene.scene.mov[num].x;
    SYS_ENV_SetU32(name,data);
    //
    sprintf(name, "SceneMov%dY",num);
    data = gCurScene.scene.mov[num].y;
    SYS_ENV_SetU32(name,data);
    //
    sprintf(name, "SceneMov%dW",num);
    data =gCurScene.scene.mov[num].w;
    SYS_ENV_SetU32(name,data);
    //
    sprintf(name, "SceneMov%dH",num);
    data =gCurScene.scene.mov[num].h;
    SYS_ENV_SetU32(name,data);
    //save
    SYS_ENV_Save();
    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_GetMovArea(WV_U32 x,WV_U32 y,WV_U32 w,WV_U32 h);
*******************************************************************/
WV_S32 TSK_SCENE_GetMovArea(WV_U32 num,WV_U32 *pEna,WV_U32* pX,WV_U32* pY,WV_U32* pW,WV_U32 *pH)
{

    *pEna = gCurScene.scene.mov[num].ena;
    *pX = gCurScene.scene.mov[num].x;
    *pY = gCurScene.scene.mov[num].y;
    *pW = gCurScene.scene.mov[num].w;
    *pH = gCurScene.scene.mov[num].h;

    //printf("num[%d]x%d,y%d,w%d,h%d\n",num,gCurScene.scene.mov[num].x,gCurScene.scene.mov[num].y,gCurScene.scene.mov[num].w,gCurScene.scene.mov[num].h);
    return WV_SOK;
} 
/*******************************************************************
 WV_S32 TSK_SCENE_ConfMov();
*******************************************************************/
WV_S32 TSK_SCENE_ConfMov(WV_U32 num)
{
    WV_S32 i;
    WV_S8  name[WV_CONF_NAME_MAX_LEN];
    //for movi
    sprintf(name, "./mov/mov%d.mp4",gCurScene.scene.mov[num].id);
    if(access(name,0)!=0){

        return WV_SOK;
    }

    TSK_PLAYER_setWin(num,gCurScene.scene.mov[num].x,gCurScene.scene.mov[num].y,gCurScene.scene.mov[num].w,gCurScene.scene.mov[num].h);
    TSK_PLAYER_Destory(num);
    WV_U8 Fade = 0;
    WV_S32 Ret = WV_EFAIL;

    if (TSK_CONF_LoadFadeinout(&Fade) == WV_SOK)
    {
        if (Fade == 1)
        {
            if (num == 0)
            {
                Ret = TSK_PLAYER_VidFrameToPicture(0);
            }
            else if (num == 1)
            {
                Ret = TSK_PLAYER_VidFrameToPicture(1);
            }
        }
    }

    if(gCurScene.scene.mov[num].ena ==1)//if ena ==1 player ;if ena ==0 stop player
    {
        //printf("++++++++++ gCurScene.scene.mov[%d].ena  = %d\n",num,gCurScene.scene.mov[num].ena);
        TSK_PLAYER_SetFile(num,name);
        //printf("in TSK_SCENE_ConfMov used TSK_PLAYER_SetFile+++++++++++++++++\n");
        TSK_PLAYER_setWin(num,gCurScene.scene.mov[num].x,gCurScene.scene.mov[num].y,gCurScene.scene.mov[num].w,gCurScene.scene.mov[num].h);
        //printf("in TSK_SCENE_ConfMov used TSK_PLAYER_setWin +++++++++++++++\n");
        TSK_PLAYER_Creat(num);
        //printf("in TSK_SCENE_ConfMov used TSK_PLAYER_Creat+++++++++++++\n");
    }
    //printf("in TSK_SCENE_ConfMov bufore  TSK_PLAYER_ReadVolume +++++++++++++\n");
    if (Ret == WV_SOK)
    {
        TSK_SCENE_SetAlpha();
    }
    TSK_PLAYER_ReadVolume();
    //printf("in TSK_SCENE_ConfMov under  TSK_PLAYER_ReadVolume +++++++++++++\n");
    return WV_SOK;
}

/*******************************************************************
WV_S32 TSK_SCENE_SetAni(WV_U32    num,WV_U32    id,WV_U32    x,WV_U32    y,WV_U32    w,WV_U32 h);
*******************************************************************/
WV_S32 TSK_SCENE_SetAni(WV_U32    num,WV_U32    id,WV_U32    x,WV_U32    y,WV_U32    w,WV_U32 h)
{
    if(num >= HIS_HIGO_GODEC_NUM ) return WV_EFAIL;
    gCurScene.scene.animation[num].id = id;
    gCurScene.scene.animation[num].x = x;
    gCurScene.scene.animation[num].y = y;
    gCurScene.scene.animation[num].w = w;
    gCurScene.scene.animation[num].h = h;

    //WV_CHECK_RET(TSK_SCENE_ConfAni());
    return WV_SOK;
}  

/*******************************************************************
WV_S32 TSK_SCENE_SetAniNum(WV_U32    num);
*******************************************************************/

WV_S32 TSK_SCENE_SetAniNum(WV_U32    num)
{
    if(num > HIS_HIGO_GODEC_NUM ) return WV_EFAIL;
    gCurScene.scene.animationNum  = num;
    //WV_CHECK_RET(TSK_SCENE_ConfAni());
    return WV_SOK;
}
/*******************************************************************
WV_S32 TSK_SCENE_ConfAni();
*******************************************************************/
WV_S32 TSK_SCENE_ConfAni()
{
    WV_S32 i;
    
    WV_S8 pName[30];
    for(i = 0;i< HIS_HIGO_GODEC_NUM ; i++ )
    {
        TSK_GO_DecClose(i);
    }

    for(i=0;i< gCurScene.scene.animationNum;i++ )
    {
        sprintf(pName,"./gif/%d.gif",gCurScene.scene.animation[i].id);
        //sprintf(pName,"/usb/pic/fengjing.jpg");
        TSK_GO_DecOpen(i,pName,gCurScene.scene.animation[i].x,\
                       gCurScene.scene.animation[i].y,gCurScene.scene.animation[i].w,gCurScene.scene.animation[i].h);
    }

    return WV_SOK;
}

/*******************************************************************
WV_S32 TSK_SCENE_CleanWinFram();
*******************************************************************/
WV_S32 TSK_SCENE_CleanWinFram()
{

    TSK_SCENE_ConfWin();

    return WV_SOK;
}
/*******************************************************************
WV_S32 TSK_SCENE_SetWinNum(WV_U32    num);
*******************************************************************/

WV_S32 TSK_SCENE_SetWinNum(WV_U32    num)
{
    if(num > TSK_SCENE_WIN_MAX_NUM ) return WV_EFAIL;
    gCurScene.scene.winNum  = num;

    //WV_ASSERT_RET ( TSK_SCENE_ConfWin());
    return WV_SOK;
}
/*******************************************************************

WV_S32 TSK_SCENE_DeletLastWin();
删除红外开窗
*******************************************************************/
WV_S32 TSK_SCENE_DeletLastWin()
{

    //if(gCurScene.scene.winNum == 0 )  return  WV_EFAIL;
    
    if(gCurScene.addOutline == 1 && gCurScene.scene.winNum >=1)
    {
        gCurScene.scene.winNum --;
        TSK_SCENE_ConfWin();
        gCurScene.addOutline = 0;
    }

    if(gCurScene.addAni == 1 && gCurScene.scene.animationNum >=1)
    {
        gCurScene.scene.animationNum --;
        TSK_SCENE_ConfAni();
        gCurScene.addAni = 0;
    }
    return WV_SOK;
}
/*******************************************************************

WV_S32 TSK_SCENE_AddLastWin();
添加红外开窗
*******************************************************************/
WV_S32 TSK_SCENE_AddLastWin()
{
    TSK_GO_MOV_PIC_POS_S pos;
    WV_U16 chn;
    WV_U16 id;
    WV_U16 backAniEna;
    if(gCurScene.scene.winNum >= TSK_SCENE_WIN_MAX_NUM )  return  WV_EFAIL;


    if(TSK_GO_GetOutline_Pos(&pos,&chn,&id) != 0 )
    {
        return WV_EFAIL;
    }

    if(pos.startX==0 && pos.startY==0 && pos.endX==0 && pos.endY== 0)
    {
        return WV_EFAIL;
    }

    if(gCurScene.addOutline == 0)
    {

        gCurScene.scene.win[gCurScene.scene.winNum].outId = id;
        gCurScene.scene.win[gCurScene.scene.winNum].x = pos.startX;
        gCurScene.scene.win[gCurScene.scene.winNum].y = pos.startY;
        gCurScene.scene.win[gCurScene.scene.winNum].w = pos.endX - pos.startX;
        gCurScene.scene.win[gCurScene.scene.winNum].h = pos.endY - pos.startY;
        gCurScene.scene.win[gCurScene.scene.winNum].videoId = chn;
        gCurScene.scene.winNum ++;
        TSK_SCENE_ConfWin();
        gCurScene.addOutline = 1;
    }
    //添加开窗背景图片

    if(gCurScene.scene.animationNum >= TSK_SCENE_GIF_MAX_NUM)  return  WV_EFAIL;

    if(TSK_GO_GetBackAni_Pos(&pos,&chn,&id) != 0 )
    {
        return WV_EFAIL;
    }

    if(gCurScene.addAni == 0)
    {
        gCurScene.scene.animation[gCurScene.scene.animationNum].id = id;
        gCurScene.scene.animation[gCurScene.scene.animationNum].x = pos.startX;
        gCurScene.scene.animation[gCurScene.scene.animationNum].y = pos.startY;
        gCurScene.scene.animation[gCurScene.scene.animationNum].w = pos.endX - pos.startX;
        gCurScene.scene.animation[gCurScene.scene.animationNum].h = pos.endY - pos.startY;
        gCurScene.scene.animation[gCurScene.scene.animationNum].ena = 1;
        gCurScene.scene.animationNum ++;
        TSK_SCENE_ConfAni();
        gCurScene.addAni = 1;
    }

    return WV_SOK;

}
/*******************************************************************
WV_S32 TSK_SCENE_SetWin(WV_U32    num,WV_U32    id,WV_U32    x,WV_U32    y,WV_U32    w,WV_U32 h,WV_U32 vid);
*******************************************************************/
WV_S32 TSK_SCENE_SetWin(WV_U32    num,WV_U32    id,WV_U32    x,WV_U32    y,WV_U32    w,WV_U32 h,WV_U32 vid)
{
    if(num >= TSK_SCENE_WIN_MAX_NUM )  return  WV_EFAIL;
    
    if(x==0 && y==0 && w==0 && h== 0)
    {
        return WV_EFAIL;
    }
    gCurScene.scene.win[num].outId = id;
    gCurScene.scene.win[num].x = x;
    gCurScene.scene.win[num].y = y;
    gCurScene.scene.win[num].w = w;
    gCurScene.scene.win[num].h = h;
    gCurScene.scene.win[num].videoId = vid;
    //    printf("TSK_SCENE_SetWin id is %d\n",gCurScene.scene.win[num].outId);
    //    printf("TSK_SCENE_SetWin x  is %d\n",gCurScene.scene.win[num].x);
    //    printf("TSK_SCENE_SetWin y  is %d\n",gCurScene.scene.win[num].y);
    //    printf("TSK_SCENE_SetWin w  is %d\n",gCurScene.scene.win[num].w);
    //    printf("TSK_SCENE_SetWin h  is %d\n",gCurScene.scene.win[num].h);
    //    printf("TSK_SCENE_SetWin vid is %d\n",gCurScene.scene.win[num].videoId);
    return WV_SOK;
}
/*******************************************************************
WV_S32 TSK_SCENE_ConWin();
*******************************************************************/
WV_S32 TSK_SCENE_ConfWin()
{
    WV_ASSERT_RET ( FPGA_CONF_SetWin (gCurScene.scene.winNum ,gCurScene.scene.win));
    return WV_SOK;
}  

/*******************************************************************
WV_S32 TSK_SCENE_CloseWinAndAni();
*******************************************************************/
WV_S32 TSK_SCENE_CloseWinAndAni()
{
    WV_S32 i;
    for(i = 0;i< HIS_HIGO_GODEC_NUM ; i++ )
    {
        TSK_GO_DecClose(i);
    }

    TSK_SCENE_SetWinNum(0);
    TSK_SCENE_ConfWin();


    return WV_SOK;
}

/*******************************************************************
WV_S32 TSK_SCENE_OpenWinAndAni();
*******************************************************************/
WV_S32 TSK_SCENE_OpenWinAndAni()
{
    WV_S32 i;
    WV_S8 name[20];
    WV_U32 data;

    sprintf(name, "Scene%dwinNum",gCurScene.id);
    SYS_ENV_GetU32(name,&data);
    gCurScene.scene.winNum  = data;
    TSK_SCENE_SetWinNum(gCurScene.scene.winNum);
    TSK_SCENE_ConfWin();

    TSK_SCENE_ConfAni();
    return WV_SOK;
}    
/*******************************************************************

WV_S32 TSK_SCENE_SaveAngle(WV_U32 dataLen,WV_U8 *pData);

*******************************************************************/
WV_S32 TSK_SCENE_SaveAngle(WV_U32 dataLen,WV_U8 *pData)
{

    WV_U32 len;
    FILE *fp;

    fp=fopen(TSK_SCENE_OUTPUT_ANGLE_FILE,"wb+");
    if(fp == NULL)
    {
        WV_printf("fopen %s error \n",TSK_SCENE_OUTPUT_ANGLE_FILE);
        return WV_EFAIL;
    }
    len = fwrite(pData,1,dataLen,fp);
    if(len != dataLen)
    {
        WV_printf("fwrite %s error \n",TSK_SCENE_OUTPUT_ANGLE_FILE);
        fclose(fp);
        return WV_EFAIL;
    }

    fclose(fp);
    system("sync");


    return WV_SOK;
}    

/*******************************************************************

WV_S32 TSK_SCENE_GetAngle(WV_U32 dataLen,WV_U8 *pData);

*******************************************************************/
WV_S32 TSK_SCENE_GetAngle(WV_U32 *pDataLen,WV_U8 *pData)
{

    WV_S32 readLen=0;
    FILE *fp;
    WV_U8 *p=pData;
    fp=fopen(TSK_SCENE_OUTPUT_ANGLE_FILE,"rb");
    if(fp == NULL)
    {
        WV_printf("fopen %s error \n",TSK_SCENE_OUTPUT_ANGLE_FILE);
        return WV_EFAIL;
    }
    while(1)
    {
        readLen = fread(p,1,1024,fp);
        *pDataLen += readLen;
        p+=readLen;
        if(readLen <1024)
        {
            break;
        }
    }

    fclose(fp);
    //system("sync");
    return WV_SOK;
}
/****************************************************************************

WV_S32 TSK_SCENE_CMDGet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_SCENE_CMDGet(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{

    WV_S32 ret = 0,i;
    WV_U32 sceneId;
    TSK_SCENE_INFO_S  * pScene;
    TSK_SCENE_INFO_S   sceneTemp;


    pScene = &gCurScene.scene;
    sceneId = gCurScene.id;

    if(argc > 0 )
    {
        ret = WV_STR_S2v(argv[0],&sceneId);
        if(ret != WV_SOK  || sceneId>= TSK_SCENE_MAX_NUM  )
        {
            prfBuff += sprintf(prfBuff,"input ID  erro !\r\n");
            return WV_SOK;
        }
        pScene = &sceneTemp;
        TSK_SCENE_GetScene(sceneId ,pScene);
        
    }
    
    printf("sceneId = %d\r\n",sceneId);
    prfBuff += sprintf(prfBuff,"------------------------------------\r\n");
    prfBuff += sprintf(prfBuff,"Sence  ID         = %u \r\n",sceneId);
    prfBuff += sprintf(prfBuff,"Sence  mov  id    = %u \r\n",pScene->mov[0].id);
    prfBuff += sprintf(prfBuff,"Sence  mov   x    = %u \r\n",pScene->mov[0].x);
    prfBuff += sprintf(prfBuff,"Sence  mov   y    = %u \r\n",pScene->mov[0].y);
    prfBuff += sprintf(prfBuff,"Sence  mov   w    = %u \r\n",pScene->mov[0].w);
    prfBuff += sprintf(prfBuff,"Sence  mov   h    = %u \r\n",pScene->mov[0].h);

    prfBuff += sprintf(prfBuff,"------------------------------------\r\n");
    prfBuff += sprintf(prfBuff,"Sence  ani num    = %u \r\n",pScene->animationNum );
    for(i=0;i< pScene->animationNum; i++)
    {
        prfBuff += sprintf(prfBuff,"Sence ani[%d] id    = %u \r\n",i,pScene->animation[i].id);
        prfBuff += sprintf(prfBuff,"Sence ani[%d]  x    = %u \r\n",i,pScene->animation[i].x);
        prfBuff += sprintf(prfBuff,"Sence ani[%d]  y    = %u \r\n",i,pScene->animation[i].y);
        prfBuff += sprintf(prfBuff,"Sence ani[%d]  w    = %u \r\n",i,pScene->animation[i].w);
        prfBuff += sprintf(prfBuff,"Sence ani[%d]  h    = %u \r\n",i,pScene->animation[i].h);
    }
    prfBuff += sprintf(prfBuff,"------------------------------------\r\n");
    prfBuff += sprintf(prfBuff,"Sence  win num    = %u \r\n",pScene->winNum );
    for(i=0;i< pScene->winNum; i++)
    {
        prfBuff += sprintf(prfBuff,"Sence win[%d]  id    = %u \r\n",i,pScene->win[i].outId);
        prfBuff += sprintf(prfBuff,"Sence win[%d]   x    = %u \r\n",i,pScene->win[i].x);
        prfBuff += sprintf(prfBuff,"Sence win[%d]   y    = %u \r\n",i,pScene->win[i].y);
        prfBuff += sprintf(prfBuff,"Sence win[%d]   w    = %u \r\n",i,pScene->win[i].w);
        prfBuff += sprintf(prfBuff,"Sence win[%d]   h    = %u \r\n",i,pScene->win[i].h);
        prfBuff += sprintf(prfBuff,"Sence win[%d] vid    = %u \r\n",i,pScene->win[i].videoId);
    }

    return WV_SOK;
}

/****************************************************************************

WV_S32 TSK_SCENE_CMDSet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_SCENE_CMDSet(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{

    WV_U32 num,id,x,y,w,h,vid;


    if(argc <1  )
    {
        prfBuff += sprintf(prfBuff,"set scene  <cmd>  cmd : mov /ani/aninum/win/winnum/winset/winclr/save/change/reset/url/playsync\r\n");
        return WV_SOK;
    }
    
    if(strcmp(argv[0],"mov") ==0)
    {
        if(argc < 2 )
        {
            prfBuff += sprintf(prfBuff,"set scene  mov  < ID >\r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&id);

        TSK_SCENE_SetMov(0,id);
        TSK_SCENE_Restart();
        return WV_SOK;
    }
    
    if(strcmp(argv[0],"ani") ==0)
    {
        if(argc < 7)
        {
            prfBuff += sprintf(prfBuff,"set scene  ani <num>  < ID > < X > < Y > < W > < H >\r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&num);
        WV_STR_S2v(argv[2],&id);
        WV_STR_S2v(argv[3],&x);
        WV_STR_S2v(argv[4],&y);
        WV_STR_S2v(argv[5],&w);
        WV_STR_S2v(argv[6],&h);
        WV_ASSERT_RET ( TSK_SCENE_SetAni(num,id,x,y,w,h));
        return WV_SOK;
    }


    if(strcmp(argv[0],"aninum") ==0)
    {
        if(argc < 2)
        {
            prfBuff += sprintf(prfBuff,"set scene  aninum  <num> \r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&num);
        WV_ASSERT_RET ( TSK_SCENE_SetAniNum(num));
        return WV_SOK;
    }
    if(strcmp(argv[0],"winnum") ==0)
    {
        if(argc < 2)
        {
            prfBuff += sprintf(prfBuff,"set scene  winnum  <num> \r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&num);
        WV_ASSERT_RET ( TSK_SCENE_SetWinNum(num));
        return WV_SOK;
    }


    if(strcmp(argv[0],"win") ==0)
    {
        if(argc < 8)
        {
            prfBuff += sprintf(prfBuff,"set scene  win <num>  < outlineId > < X > < Y > < W > < H > < vid>\r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&num);
        WV_STR_S2v(argv[2],&id);
        WV_STR_S2v(argv[3],&x);
        WV_STR_S2v(argv[4],&y);
        WV_STR_S2v(argv[5],&w);
        WV_STR_S2v(argv[6],&h);
        WV_STR_S2v(argv[7],&vid);
        WV_ASSERT_RET ( TSK_SCENE_SetWin(num,id,x,y,w,h,vid));
        return WV_SOK;
    }

    if(strcmp(argv[0],"winset") ==0)
    {

        WV_ASSERT_RET ( FPGA_CONF_SetWin (gCurScene.scene.winNum ,gCurScene.scene.win));
        return WV_SOK;
    }

    if(strcmp(argv[0],"winclr") ==0)
    {

        WV_ASSERT_RET ( FPGA_CONF_SetWin (0,gCurScene.scene.win));
        return WV_SOK;
    }

    if(strcmp(argv[0],"change") ==0)
    {
        if(argc < 2)
        {
            prfBuff += sprintf(prfBuff,"set scene  change  < id >\r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&num);
        WV_ASSERT_RET (TSK_SCENE_Change(TSK_SCENE_TYPE_NETDATA, num));
        return WV_SOK;
    }
    if(strcmp(argv[0],"save") ==0)
    {
        if(argc < 2)
        {
            prfBuff += sprintf(prfBuff,"set scene  change  < id >\r\n");
            return WV_SOK;
        }
        WV_STR_S2v(argv[1],&id);
        WV_ASSERT_RET ( TSK_SCENE_SaveScene(id ,&gCurScene.scene));
        return WV_SOK;
    }
    if(strcmp(argv[0],"reset") ==0)
    {

        WV_ASSERT_RET (TSK_SCENE_Restart());
        return WV_SOK;
    }
    if(strcmp(argv[0],"url") ==0)
    {
        if(strcmp(argv[1],"0") == 0)
        {
            WV_S8 url[64];
            //sprintf(url,"http://192.168.1.20/mov1.mp4");
            //TSK_SCENE_PlayMovUrl(2,url,28);
        }
        else if(strcmp(argv[1],"1") == 0)
        {
            WV_S8 url[64];
            sprintf(url,"./mov/mov0.mp4");
            TSK_SCENE_PlayMovUrl(2,url,14);
        }

        return WV_SOK;
    }

    if(strcmp(argv[0],"playsync") ==0)
    {
        WV_U32 syncEna;
        if(argc < 2)
        {
            prfBuff += sprintf(prfBuff,"set scene  playsync  ena[0,1]\r\n");
            return WV_SOK;
        }

        WV_STR_S2v(argv[1],&syncEna);
        if(syncEna != 0 && syncEna != 1 )
        {
            return WV_SOK;
        }

        gCurScene.DevCascading = syncEna;

        return WV_SOK;
    }


    return WV_SOK;
}

/*******************************************************************
 WV_S32 TSK_SCENE_Init();
*******************************************************************/

WV_S32  TSK_SCENE_Init()
{

    WV_S32 ret = 0;
    //先注册scene命令到控制台
    WV_CMD_Register("get","scene","print scene info",TSK_SCENE_CMDGet);
    WV_CMD_Register("set","scene","set scene ",TSK_SCENE_CMDSet);
    WV_ASSERT_RET (TSK_SCENE_RegisterConf());
    WV_ASSERT_RET (TSK_SCENE_GetConf());
    WV_ASSERT_RET (TSK_SCENE_SceneInit());

    TSK_CONF_MovInfo_Init();
    TSK_CONF_SceneInfo_Init();

    //memset(&gPlayerSync,0,sizeof(gPlayerSync));

    //WV_CHECK_RET( WV_THR_Create(&(gPlayerSync.thrHndl),TSK_SCENE_PlaySync_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, (void *)&gPlayerSync));
    gettimeofday(&SceneChangeTimeStart, NULL);
    SceneChangeDataType = TSK_SCENE_TYPE_NETDATA;

    return ret;
}

/*******************************************************************
 WV_S32 TSK_SCENE_DeInit();
*******************************************************************/ 
WV_S32  TSK_SCENE_DeInit()
{

    //gPlayerSync.close =1 ;
    TSK_CONF_SceneInfo_DeInit();
    TSK_CONF_MovInfo_DeInit();
    WV_S32 ret = 0,i;
    for(i=0;i< TSK_SCENE_MOV_USE_NUM;i++)
    {
        TSK_PLAYER_setWin(i,0,0,0,0);
    }
    WV_ASSERT_RET (TSK_SCENE_SceneClose());

    TSK_PLAYER_Destory(2);
    return ret;
}