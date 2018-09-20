// crc.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "crc.h"

WORD CRC::CalCrcFast(char* puchMsg,WORD usDataLen)
{
		BYTE uchCRCHi = 0xFF ;  /* CRC���ֽڳ�ʼ�� */
        BYTE uchCRCLo = 0xFF ;  /* CRC���ֽڳ�ʼ�� */
        WORD uIndex ;           /* CRC��������*/
                   
        while (usDataLen--)             
        {
                  uIndex = uchCRCHi ^ *puchMsg++ ;       /* ���� CRC */
                  uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
                  uchCRCLo = auchCRCLo[uIndex] ;
        }
        return (uchCRCHi << 8 | uchCRCLo) ; 
}