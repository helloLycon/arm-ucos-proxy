#ifndef __WEBSERVER_FORM_STRING_H
#define __WEBSERVER_FORM_STRING_H



#if  0
//#define  tot_strlen(x)  ((x)*8+1)
/* ����ʧ�� */
#if 0
#define  FORM_CONFIG_FAILURE  "&#37197;&#32622;&#22833;&#36133;"
#else
#define FORM_CONFIG_FAILURE "<span class=\"label label-important\">&#37197;&#32622;&#22833;&#36133;&#65281;</span>"
#endif
/* �˿ڿ��� */
//����
#define FORM_ENABLE  "&#21551;&#29992;" 
//�ض�
#define FORM_DISABLE "&#20851;&#26029;" 

/* ����/�ر� */
//����
#define FORM_OPEN  "&#24320;&#21551;"
//�ر�
#define FORM_CLOSE "&#20851;&#38381;"


/* ����״̬ */
//��������
#define FORM_LOOP_NORMAL  "&#27491;&#24120;&#24037;&#20316;"
//��·����
#define FORM_LOOP_CIRCUIT "&#32447;&#36335;&#20391;&#29615;&#22238;"
//�豸����
#define FORM_LOOP_DEVICE  "&#35774;&#22791;&#20391;&#29615;&#22238;"
//˫�򻷻�
#define FORM_LOOP_DOUBLE  "&#21452;&#21521;&#29615;&#22238;"

/* �澯���� */
//����澯�ϱ�
#define FORM_TRAP_NOMASK "&#20801;&#35768;&#21578;&#35686;&#19978;&#25253;"
//���Ƹ澯�ϱ�
#define FORM_TRAP_MASK   "&#25233;&#21046;&#21578;&#35686;&#19978;&#25253;"

/* los,lof״̬�� */
//�澯
#define FORM_STATE_WARN   "&#21578;&#35686;"
//����
#define FORM_STATE_NORMAL "&#27491;&#24120;"

/* link */
//������
#define FORM_LINK   "&#24050;&#36830;&#25509;"
//δ����
#define FORM_UNLINK "&#26410;&#36830;&#25509;"

/* �Զ�Э�� */
//ǿ��
#define FORM_FORCE  "&#24378;&#21046;"
//�Զ�Э
#define FORM_AUTO_NEGO "&#33258;&#21160;&#21327;&#21830;"

/* �˿����� */
#define FORM_10M   "10M"
#define FORM_100M  "100M"
#define FORM_1000M "1000M"

/* ȫ��˫�� */
//ȫ˫��
#define FORM_DUPLEX_FULL "&#20840;&#21452;&#24037;"
//��˫��
#define FORM_DUPLEX_HALF "&#21322;&#21452;&#24037;"

#endif


// ����/�ض�
//#define CGISTR_ENABLE      "%26%2321551"  
//#define CGISTR_DISABLE     "%26%2320851"  // windows-1252
#define CGISTR_ENABLE      "%E5%90%AF"      // utf-8
#define CGISTR_DISABLE     "%E5%85%B3"
// ����/�ر�
//#define CGISTR_OPEN   "%26%2324320"
//#define CGISTR_CLOSE  "%26%2320851"
#define CGISTR_OPEN   "%E5%BC%80"
#define CGISTR_CLOSE  "%E5%85%B3"
//�澯����
//#define CGISTR_TRAP_MASK   "%26%2325233"
//#define CGISTR_TRAP_NOMASK "%26%2320801"
#define CGISTR_TRAP_MASK   "%E6%8A%91"
#define CGISTR_TRAP_NOMASK "%E5%85%81"
//����
#define CGISTR_LOOP_NORMAL  "%E6%AD%A3"
#define CGISTR_LOOP_CIRCUIT "%E7%BA%BF"
#define CGISTR_LOOP_DEVICE  "%E8%AE%BE"
#define CGISTR_LOOP_DOUBLE  "%E5%8F%8C"
//ǿ��/�Զ�Э��
#define CGISTR_FORCE     "%E5%BC%BA"
#define CGISTR_AUTO_NEGO "%E8%87%AA"
// ȫ/��˫��
#define CGISTR_DUPLEX_HALF "%E5%8D%8A"
#define CGISTR_DUPLEX_FULL "%E5%85%A8"

#endif

