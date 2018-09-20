// crc.cpp : 实现文件
//

#include "stdafx.h"
#include "crc.h"

WORD CRC::CalCrcFast(char* puchMsg,WORD usDataLen)
{
		BYTE uchCRCHi = 0xFF ;  /* CRC高字节初始化 */
        BYTE uchCRCLo = 0xFF ;  /* CRC低字节初始化 */
        WORD uIndex ;           /* CRC查表的索引*/
                   
        while (usDataLen--)             
        {
                  uIndex = uchCRCHi ^ *puchMsg++ ;       /* 计算 CRC */
                  uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
                  uchCRCLo = auchCRCLo[uIndex] ;
        }
        return (uchCRCHi << 8 | uchCRCLo) ; 
}