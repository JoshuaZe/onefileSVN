#include<iostream>
#include<fstream>
#include<windows.h>
#include<conio.h>
using namespace std;
const int MaxVersionNum=60;//�����ð汾��
const int MaxFileNum=50;   //���ɿ��ļ���

struct FileData        //��ص��ļ�����
{
	char title[50];    //�ļ���
	long time;         //����ʱ��
};
struct FileAndStatus   //�ļ���������ļ�������
{
	FileData data;     //����ļ�������
	char status;       //'0'����û�иı�
};
struct Version_Info              //�汾��Ϣ
{
	int version;                 //��ǰ�汾��
	int lastversion;             //��һ�汾��
	FileData data[MaxFileNum];   //�����汾�ļ���Ϣ
	char log[50];                //���ɰ汾����Ϣ
};
//�жϵ�ǰ�ļ����Ƿ񱻿���
bool ControlledOrNot()
{
	HANDLE hFileTemp;
	WIN32_FIND_DATA FileDataTemp;

	hFileTemp=FindFirstFile("_svn_",&FileDataTemp);
	if(hFileTemp==INVALID_HANDLE_VALUE)
	{
		return false;         //�ļ���δ������
	}
	return true;              //�ļ����ѱ�����
}
//�ж��Ƿ����update
bool CanUpdate(FileAndStatus fData[],int total)
{
	for(int i=0;i<=total-1;i++)
	{
		if(fData[i].status!='0'&&fData[i].status!='?')//���ֻ��?��0����Խ���update
		return false;
	}
	return true;
}
//�������а汾����Ϣ
int Load(Version_Info TotalVersion[])
{
	int NumVersion;
	//��λ��ǰĿ¼
	char Directory[255];               //��¼��ǰ���̵ĵ�ǰĿ¼
	GetCurrentDirectory(255,Directory);//�ҵ���ǰ���̵ĵ�ǰĿ¼
	//�����ļ������ڴ�
	ifstream infile(strcat(Directory,"\\_svn_\\config.dat"),ios::in);
	if(!infile)                        //����ʹ�ü��ļ���ʧ
	{
		ofstream outfile(Directory,ios::out);
		NumVersion=0;
		outfile.write((char*)&NumVersion,sizeof(NumVersion));
		outfile.close();
		return NumVersion;
	}
	infile.read((char*)&NumVersion,sizeof(NumVersion));
	for(int temp=0;temp<NumVersion;temp++)
	{
		infile.read((char*)&(TotalVersion[temp]),sizeof(TotalVersion[temp]));
	}
	infile.close();
	return NumVersion;                 //�����ܰ汾���������°汾����
}
//�������°汾����Ϣ���汾�����
void LoadVersion(Version_Info TotalVersion[],int version,FileAndStatus CurrentVersion[])
{
	int temp=0;
	if(version==0)
	{
		for(;temp<MaxFileNum;temp++)
		{
			CurrentVersion[temp].data=TotalVersion[version].data[temp];
		}
	}
	else
	{
		for(;temp<MaxFileNum;temp++)
		{
			CurrentVersion[temp].data=TotalVersion[version-1].data[temp];
		}
	}	
}
//ɨ���ļ����ļ�ͬʱ���бȶ�
int ScanFile(FileAndStatus fData[],FileAndStatus backup[],FileAndStatus operation[])
{
	WIN32_FIND_DATA datatemp;//��ʱ����
	HANDLE hSearch;          //�����������
	int isadded;
	int total=0;             
                             //ɨ���ļ���
    hSearch=FindFirstFile("*",&datatemp);
    do{
        if(strcmp(datatemp.cFileName,"svn.exe")&&strcmp(datatemp.cFileName,".")&&strcmp(datatemp.cFileName,"..")&&
			(datatemp.dwFileAttributes!= FILE_ATTRIBUTE_DIRECTORY))	
		{
			fData[total].data.time=datatemp.ftLastWriteTime.dwLowDateTime;
			strcpy(fData[total].data.title,datatemp.cFileName);   
            ++total;
		}
    }while(FindNextFile(hSearch,&datatemp));
	FindClose(hSearch);


	for(int j=0;backup[j].data.title[0]!=NULL;j++) backup[j].status='N';
	for(int i=0;i<=total-1;i++)
	{
		isadded=false;
		for(j=0;backup[j].data.title[0]!=NULL;j++)
		{
			if((strcmp(fData[i].data.title,backup[j].data.title))==0)//��'0'��'M'��������ж�
			{
				isadded=true;
				backup[j].status='Y';
				fData[i].status='0';
				if(fData[i].data.time!=backup[j].data.time)
				fData[i].status='M';			
			}
		}
		if(isadded==false)                                           //'?'�����
	    fData[i].status='?';
		for(j=0;operation[j].data.title[0]!=NULL;j++)                //'-'��'+'�����
		{
			if((strcmp(fData[i].data.title,operation[j].data.title))==0)
				if(fData[i].status!='G')
					fData[i].status=operation[j].status;
		}

	}	
	                                                                 //'G'�����
	for(j=0;backup[j].data.title[0]!=NULL;j++)
	{
		if(backup[j].status!='Y')
		{
		    fData[total].data.time=backup[j].data.time;
			strcpy(fData[total].data.title,backup[j].data.title); 
			fData[total].status='G';
			total++;
		}
	}	
	fData[total].data.time=0;
	return total;                                                    //�����ļ�����
}


//����_svn_�ļ���
bool create()
{
	//��λ��ǰĿ¼
	char Directory[255];               //��¼��ǰ���̵ĵ�ǰĿ¼
	GetCurrentDirectory(255,Directory);//�ҵ���ǰ���̵ĵ�ǰĿ¼
	//����_svn_�ļ���
	system("mkdir _svn_");
	system("mkdir _svn_\\data");
	return true;
}
//��ʾ�ļ����ļ��ܿ�״̬
void show_status(FileAndStatus fData[],int TotalFile)
{
	for(int i=0;i<=TotalFile-1;i++)
	if(fData[i].status!='0')
		cout<<fData[i].status<<"  "<<fData[i].data.title<<endl;
}
//����ܿ��ļ���?->+��
bool add(FileAndStatus fData[],int total,char *name,FileAndStatus operation[])
{
	int i,j=0;                                //ѭ������
	for(i=0;i<=total-1;i++)
	{
		if(strcmp(fData[i].data.title,name)==0&&(fData[i].status=='?'))
		{
			fData[i].status='+';              //�޸�״̬
			while(operation[j].data.title[0]!=NULL)//�޸Ĳ�����¼
			j++;
			strcpy(operation[j].data.title,name);
			operation[j].status='+';
			return true;                      //�޸ĳɹ�
		}
	}
	return false;                             //�޸�ʧ��
}
//�ܿ��ļ��Ƴ�Ŀ¼��+&0->-��
bool del(FileAndStatus fData[],int total,char *name,FileAndStatus operation[])
{
	int i,j=0;                                 //ѭ������
	for(i=0;i<=total-1;i++)
	{
		if(strcmp(fData[i].data.title,name)==0&&((fData[i].status=='0')||(fData[i].status=='+')||(fData[i].status=='M')))
		{
			fData[i].status='-';              //�޸�״̬
		    while(operation[j].data.title[0]!=NULL&&strcmp(operation[j].data.title,name)!=0)//�޸Ĳ�����¼
			j++;
			strcpy(operation[j].data.title,name);
			operation[j].status='-';
		    return true;                       //�޸ĳɹ�
		}          
	}
	return false;                              //�޸�ʧ��
}\

//�ع���ĳһ�汾����Ĭ������
int update(Version_Info TotalVersion[],int To_version,int total,FileAndStatus fData[])
{
	int i=0;
	char szPath[MAX_PATH];   //szPath[]��ʾ��ǰĿ¼·��
    char szPath_Cur[MAX_PATH],szPath_Des[MAX_PATH],szPath_Del[MAX_PATH];  //szPath_Cur[],szPath_Des[]�ֱ��ʾ���Ƶ�Դ�ļ���Ŀ���ļ���·��
	char title_temp[50];     //���ڸ����ļ��Ĺ����д���ļ���������
    //�ж��Ƿ����޸ġ���ʧ��ɾ������ӵ��ļ�
	i=0;
	//��ɾ���ļ���������txt�ļ�
    GetCurrentDirectory(MAX_PATH,szPath);
    while (i<=total-1)
	{
	   strcpy(szPath_Del,szPath);
	   if(fData[i].status!='?')
       DeleteFile(strcat(strcat(szPath_Del,"\\"),fData[i].data.title));
	   i++;
	}
    //�Ӻ��ʵİ汾�ж�ȡ�ļ���Ϣ�����Ƶ��ļ��У�������
    i=0;
    while (TotalVersion[To_version-1].data[i].time!=0)
	{  
		strcpy(szPath_Cur,szPath);
		strcpy(szPath_Des,szPath);
		CopyFile(strcat(strcat(szPath_Cur,"\\_svn_\\data\\"),ltoa(TotalVersion[To_version-1].data[i].time,title_temp,10)),strcat(strcat(szPath_Des,"\\"),TotalVersion[To_version-1].data[i].title),false);
		i++;													
	}
    //���ĵ�ǰ�汾��
    return To_version;//Ҫupdate�İ汾��	
}
//ִ��
bool commit(FileAndStatus fData[],Version_Info TotalVersion[],int version,char alog[],int numversion)//alog��ʾ�û������log
{
	int i=0,v=0,j=0;
	v=numversion;
	bool IsModificated=false; //��������ļ���δ���޸ģ��򷵻�no modification�����򷵻ذ汾��
	//�ж��Ƿ����ļ����޸�
	while (!IsModificated && fData[i].data.time!=0)
	{
		if (fData[i].status=='+' || fData[i].status=='M'||fData[i].status=='-'||fData[i].status=='G')  
			IsModificated=true;
		i++;
	}
    TotalVersion[v].version=v+1;//���˳����Ǵ�0��ʼ��Ļ�
    TotalVersion[v].lastversion=version; //����ǰ�汾�Ÿ�ֵ��ȥ
    i=0;
	char szPath_Cur[MAX_PATH];
    while (fData[i].data.time!=0 && IsModificated)
	{
		GetCurrentDirectory(MAX_PATH,szPath_Cur);
		switch (fData[i].status)
		{
	      case '0'://û�޸�
			  //���ļ�����ʱ�䳤�ȴ����ڴ�������
			  TotalVersion[v].data[j].time =fData[i].data.time;
			  strcpy(TotalVersion[v].data[j].title,fData[i].data.title);
			  j++;
			  break;
          case '+':
		  case 'M':
              //���ļ�����ʱ�䳤�ȴ����ڴ�������
			  TotalVersion[v].data[j].time =fData[i].data.time;
			  strcpy(TotalVersion[v].data[j].title,fData[i].data.title);
			  j++;
			  //���ļ����Ƶ�_svn_�ļ�����
			  char szPath_Des[MAX_PATH]; //szPath_Cur��ʾԴ�ļ�·����szPath_Des��ʾĿ��·����ַ
			  char title_temp[50];//���ڸ����ļ��Ĺ����д���ļ���������
              strcpy(szPath_Des,szPath_Cur);
              CopyFile(strcat(strcat(szPath_Cur,"\\"),fData[i].data.title),strcat(strcat(szPath_Des,"\\_svn_\\data\\"),ltoa(fData[i].data.time,title_temp,10)),false);
			  break;
		  case 'G':
		  case '-':
			  char szPath_Del[MAX_PATH];
			  strcpy(szPath_Del,szPath_Cur);
			  DeleteFile(strcat(strcat(szPath_Del,"\\"),fData[i].data.title));
			  break;
		  case '?':;
		  //case 'G':; //����Ҫ����
	   }//endswitch
		i++;
	}//endwhile
	if (IsModificated)
	{
		strcpy(TotalVersion[v].log,alog);//��¼��־log
		return true;
	}
	else
	{
		cout<<"no modification"<<endl;//���ļ�û�޸�ʱ�����"no modification"
		return false;
	}
}
//��������
void revert(FileAndStatus fData[],int total,FileAndStatus backup[])
{
	
	int i;                                                //ѭ������
	char szPath_Cur[MAX_PATH],szPath_Des[MAX_PATH];       //��¼��ַ   
	char title_temp[50];                                  //��ʱ����title��time
	long time_temp;
	for(i=0;i<=total-1;i++)
	{
		GetCurrentDirectory(MAX_PATH,szPath_Cur);        
		switch(fData[i].status)
		{
		case '+':
			//�޸ı�־
			fData[i].status='?';break; 
		case 'G':
			//��Դ�ļ����ƻ���
			fData[i].status='0';
			strcpy(szPath_Des,szPath_Cur);
			CopyFile(strcat(strcat(szPath_Des,"\\_svn_\\data\\"),ltoa(fData[i].data.time,title_temp,10)),strcat(strcat(szPath_Cur,"\\"),fData[i].data.title),false);
			break;
		case '-':
		case 'M':
			//����Դ�ļ������ļ�����
			for(int j=0;j<=49;j++)
			{
				if(strcmp(fData[i].data.title,backup[j].data.title)==0)
				{
					time_temp=backup[j].data.time;
					break;
				}
			}
			fData[i].status='0';
			strcpy(szPath_Des,szPath_Cur);
			DeleteFile(strcat(strcat(szPath_Des,"\\"),fData[i].data.title));
			strcpy(szPath_Des,szPath_Cur);
			CopyFile(strcat(strcat(szPath_Des,"\\_svn_\\data\\"),ltoa(time_temp,title_temp,10)),strcat(strcat(szPath_Cur,"\\"),fData[i].data.title),false);
			break;
		}
	}
}
//��ʾ��־
struct qstype             //log����ջ�ṹ��
{
	int VersionNum[MaxFileNum];
	int top;
};
void initiateqs(qstype *s)//logջ��ʼ��
{
	s->top=-1;
}
int push(qstype *s,int x) //log��ջ
{
	if (s->top>=MaxFileNum-1)
		return 0;
	else
	{
		s->top++;
        s->VersionNum[s->top]=x;
		return 1;
	}
}
int pop(qstype *s)        //log��ջ
{
	if (s->top<0)
		return 0;
	else
	{
		s->top--;
		return(s->VersionNum[s->top+1]);
	}
}
void output(char argument[])
{
	for(int i=0;argument[i]!='\0';i++)
	{
		if(!((i==0&&argument[i]=='"')||(argument[i+1]=='\0'&&argument[i]=='"')))   //���ͷβ�������������
		{
			cout<<argument[i];
		}
	}
		cout<<endl;
}
void log(Version_Info TotalVersion[],int *version)//����version����ʱ�����ø�log����
{
	cout<<"version: "<<*version<<endl;
	cout<<"log: ";
	output(TotalVersion[*version-1].log);
}
void log(Version_Info TotalVersion[],int nowversion)//û��version����ʱ�����ø�log����
{
	int n,version_temp=nowversion;                  //����ǰ�汾�Ÿ�ֵ��version_temp
	qstype *s;
	s=new qstype;
	initiateqs(s);
    while (nowversion!=0)
	{ 
		push(s,nowversion);                          //�汾����ջ
		nowversion=TotalVersion[nowversion-1].lastversion;
	}
    while ((n=pop(s))!='\0') 
	{
		cout<<"version: "<<n;
		if (n==version_temp) cout<<"<-";
		cout<<endl;
	    cout<<"log: ";
		output(TotalVersion[n-1].log);
		cout<<endl;
	}
}
//��ʾ��ϸ�汾������Ϣ
void attribute(Version_Info TotalVersion[],int *version)
{
	cout<<"version:"<<*version<<endl;
	cout<<"file:"<<endl;
	for(int i=0;TotalVersion[*version-1].data[i].title[0]!=NULL;i++)
	cout<<TotalVersion[*version-1].data[i].title<<endl;
	cout<<"log:";
	output(TotalVersion[*version-1].log);                     //�������
}
//����ָ����������н���
void InputOrder(char order[],char argument[])
{
	order[0]='\0';
	argument[0]='\0';
	cout<<"svn>";
	for(int i=0,j=0;1;) 
	{ 
		char temp;                //��ʱ���ַ���¼      
		temp=getch();             //�����ַ�  
		if(temp==13){break;}      //�ж�������Ƿ�س�	
		if(temp==8&&i!=0)
		{
			cout<<"\b"<<" "<<"\b";
			if(j==0){i--;argument[j]='\0';}
			else{j--;}
		}
		else if(temp==' '||j!=0)
		{
			if(j!=0)
			{
				argument[j-1]=temp;
			}
			j++;
			putchar(temp);         //�����ʱ���ַ���¼
		}
		else if(temp!=8)
		{
			order[i]=temp;
			putchar(temp);         //�����ʱ���ַ���¼
			i++;
		}
	}
	order[i]='\0';
	if(j!=0) argument[j-1]='\0';
	cout<<endl;
}

//���ַ��Ͳ���ת�������Ͱ汾��
int ChangeIntoNum(char argument[])
{
	int argunum=0;//�汾�Ų���Ϊ0����ΪĬ��
	for(int i=0;argument[i]!='\0';i++)
	{
		if(argument[0]!=48&&argument[i]>=48&&argument[i]<=57)
		{
			argunum=argunum*10+(argument[i]-48);
		}
		else{return (-1);}//�����޷�ת����
	}
	return argunum;
}
//�°汾����
void save(Version_Info TotalVersion[],int *VersionNum)
{
	ofstream outfile("_svn_\\config.dat",ios::binary);
	//�����¼����
	outfile.write((char*)&(*VersionNum),sizeof(*VersionNum));
	//�����¼
	for(int i=1;i<=*VersionNum;i++)
		outfile.write((char*)&(TotalVersion[i-1]),sizeof(TotalVersion[i-1]));
	outfile.close();
}
//��ʼ���汾��Ϣ
void initialization(Version_Info fdata[])
{
	for(int i=0;i<=MaxVersionNum-1;i++)
		for(int j=0;j<=MaxFileNum-1;j++)
		{
		    fdata[i].data[j].title[0]=NULL;
			fdata[i].data[j].time=0;
			fdata[i].log[0]=NULL;
		}
}
//��ʼ��������Ϣ
void operation_initialization(FileAndStatus temp[])
{
		for(int i=0;i<=49;i++)
		temp[i].data.title[0]=NULL;
}
int main()
{
	char order[15];                    //��������
	char argument[50];                 //�������
	int argunum;                       //����������汾��
	bool IsControlled;                 //���ļ����Ƿ񱻿���
	//��ǰ�ļ��������Ϣ
	int TotalFile;                     //��¼��ǰ�ļ����ļ�����
	FileAndStatus fData[MaxFileNum];   //�ļ����е�����ļ�����Ӧ״̬
	//�汾��Ϣ
	Version_Info TotalVersion[MaxVersionNum];//���а汾����Ϣ
	FileAndStatus CurrentVersion[MaxFileNum];//��ǰ�汾����Ϣ
	FileAndStatus OperationData[MaxFileNum]; //��¼��������Ϣ
	int NumVersion=0;                        //�汾����
	int version=0;                           //��ǰ�汾
	initialization(TotalVersion);
	operation_initialization(OperationData);
	//�жϵ�ǰ�ļ����Ƿ񱻿���
	IsControlled=ControlledOrNot();
	//�ܱ���������ļ�
	if(IsControlled)
	{
		//�������а汾����Ϣ
		NumVersion=Load(TotalVersion);
		version=NumVersion;
		//���ص�ǰ���°汾����Ϣ
		LoadVersion(TotalVersion,version,CurrentVersion);
	}
	while(1)
	{
		//����ָ����������н���
		InputOrder(order,argument);
		argunum=ChangeIntoNum(argument);
		//ָ���б𼰲���ָ���ִ��	
		if(!strcmp(order,"create")&&argument[0]=='\0')             //create�����ܱ�����־�ļ���
		{
			if(!IsControlled)
			{
				IsControlled=create();
				//���س�ʼ�汾����Ϣ
				NumVersion=Load(TotalVersion);
				//���ص�ǰ���°汾����Ϣ
				LoadVersion(TotalVersion,version,CurrentVersion);
			}
			else{cout<<"already create!"<<endl;}
		}
		else if(!strcmp(order,"quit")&&argument[0]=='\0')          //quit�˳�����
		{exit(EXIT_SUCCESS);}        
		else if(!IsControlled)                                     //ȷ���ļ��б�����
		{cout<<"please,create first!"<<endl;}
		else if(!strcmp(order,"status")&&argument[0]=='\0')        //status��ʾ�ļ��Ƚ�״̬
		{
		     	//ɨ���ļ�ͬʱ���бȶ�
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);
				show_status(fData,TotalFile);
		}
		else if(!strcmp(order,"add"))                              //add argument,����ܹ����ļ�
		{
			TotalFile=ScanFile(fData,CurrentVersion,OperationData);
			if(!add(fData,TotalFile,argument,OperationData))
			{
				cout<<"Add Error!"<<endl;
			}
		}
		else if(!strcmp(order,"delete"))                           //delete argument,ɾ���ļ�����
		{		//ɨ���ļ�ͬʱ���бȶ�
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);
				if(!del(fData,TotalFile,argument,OperationData))
			{
				cout<<"Delete Error!"<<endl;
			}
		}
		else if(!strcmp(order,"update")&&argunum!=-1)               //update argunum,������ĳһ�汾
		{
			if(argunum==0)argunum=NumVersion;
			TotalFile=ScanFile(fData,CurrentVersion,OperationData);
			if(CanUpdate(fData,TotalFile))
			{
				if(argunum>NumVersion)cout<<"The version is not existing"<<endl;
				else 
				{
					version=update(TotalVersion,argunum,TotalFile,fData);
					LoadVersion(TotalVersion,version,CurrentVersion);
					operation_initialization(OperationData);
				}
			}
			else cout<<"there changes"<<endl;
		}
		else if(!strcmp(order,"commit"))                               //commit argument,�ύ����ִ�в������Բ����°汾
		{
			TotalFile=ScanFile(fData,CurrentVersion,OperationData);
			if(commit(fData,TotalVersion,version,argument,NumVersion))
			{
				NumVersion++;
				version=NumVersion;
				cout<<"version "<<version<<endl;
				LoadVersion(TotalVersion,version,CurrentVersion);	   //���ص�ǰ���°汾����Ϣ
				save(TotalVersion,&NumVersion);                        //����汾�������ļ�
				operation_initialization(OperationData);
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);//ɨ���ļ�ͬʱ���бȶ�
			}			
		}
		else if(!strcmp(order,"revert")&&argument[0]=='\0')            //revert,��������
		{
            TotalFile=ScanFile(fData,CurrentVersion,OperationData); 
			revert(fData,TotalFile,CurrentVersion);
		    operation_initialization(OperationData);
		}
		else if(!strcmp(order,"log")&&argunum!=-1)                  //log argunum,��ʾ�汾������־
		{
			if(argunum>NumVersion)cout<<"The version is not existing"<<endl;
			else
			{
				if(argunum==0)
				{
					log(TotalVersion,version);
				}
				else
				{
					log(TotalVersion,&argunum);
				}
			}
		}
		else if(!strcmp(order,"attribute")&&argunum!=-1)            //attribute argunum,��ʾ�汾�ļ�����
		{
			if(argunum==0) argunum=version;
			attribute(TotalVersion,&argunum);
		}
		else{cout<<"Warning!No such orders."<<endl;}                //����ָ���
		cout<<endl;
	}
	return 0;
}
