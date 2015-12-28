#include<iostream>
#include<fstream>
#include<windows.h>
#include<conio.h>
using namespace std;
const int MaxVersionNum=60;//最多可用版本数
const int MaxFileNum=50;   //最多可控文件数

struct FileData        //相关的文件属性
{
	char title[50];    //文件名
	long time;         //创建时间
};
struct FileAndStatus   //文件夹中相关文件的属性
{
	FileData data;     //相关文件的属性
	char status;       //'0'代表没有改变
};
struct Version_Info              //版本信息
{
	int version;                 //当前版本号
	int lastversion;             //上一版本号
	FileData data[MaxFileNum];   //所属版本文件信息
	char log[50];                //生成版本的信息
};
//判断当前文件夹是否被控制
bool ControlledOrNot()
{
	HANDLE hFileTemp;
	WIN32_FIND_DATA FileDataTemp;

	hFileTemp=FindFirstFile("_svn_",&FileDataTemp);
	if(hFileTemp==INVALID_HANDLE_VALUE)
	{
		return false;         //文件夹未被控制
	}
	return true;              //文件夹已被控制
}
//判断是否可以update
bool CanUpdate(FileAndStatus fData[],int total)
{
	for(int i=0;i<=total-1;i++)
	{
		if(fData[i].status!='0'&&fData[i].status!='?')//如果只有?和0则可以进行update
		return false;
	}
	return true;
}
//加载所有版本的信息
int Load(Version_Info TotalVersion[])
{
	int NumVersion;
	//定位当前目录
	char Directory[255];               //记录当前进程的当前目录
	GetCurrentDirectory(255,Directory);//找到当前进程的当前目录
	//配置文件读入内存
	ifstream infile(strcat(Directory,"\\_svn_\\config.dat"),ios::in);
	if(!infile)                        //初次使用及文件丢失
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
	return NumVersion;                 //返回总版本数（即最新版本数）
}
//加载最新版本的信息（版本号最大）
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
//扫描文件夹文件同时进行比对
int ScanFile(FileAndStatus fData[],FileAndStatus backup[],FileAndStatus operation[])
{
	WIN32_FIND_DATA datatemp;//临时数据
	HANDLE hSearch;          //定义搜索句柄
	int isadded;
	int total=0;             
                             //扫描文件夹
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
			if((strcmp(fData[i].data.title,backup[j].data.title))==0)//对'0'和'M'情况进行判断
			{
				isadded=true;
				backup[j].status='Y';
				fData[i].status='0';
				if(fData[i].data.time!=backup[j].data.time)
				fData[i].status='M';			
			}
		}
		if(isadded==false)                                           //'?'的情况
	    fData[i].status='?';
		for(j=0;operation[j].data.title[0]!=NULL;j++)                //'-'和'+'的情况
		{
			if((strcmp(fData[i].data.title,operation[j].data.title))==0)
				if(fData[i].status!='G')
					fData[i].status=operation[j].status;
		}

	}	
	                                                                 //'G'的情况
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
	return total;                                                    //返回文件总数
}


//创建_svn_文件夹
bool create()
{
	//定位当前目录
	char Directory[255];               //记录当前进程的当前目录
	GetCurrentDirectory(255,Directory);//找到当前进程的当前目录
	//创建_svn_文件夹
	system("mkdir _svn_");
	system("mkdir _svn_\\data");
	return true;
}
//显示文件夹文件受控状态
void show_status(FileAndStatus fData[],int TotalFile)
{
	for(int i=0;i<=TotalFile-1;i++)
	if(fData[i].status!='0')
		cout<<fData[i].status<<"  "<<fData[i].data.title<<endl;
}
//添加受控文件（?->+）
bool add(FileAndStatus fData[],int total,char *name,FileAndStatus operation[])
{
	int i,j=0;                                //循环因子
	for(i=0;i<=total-1;i++)
	{
		if(strcmp(fData[i].data.title,name)==0&&(fData[i].status=='?'))
		{
			fData[i].status='+';              //修改状态
			while(operation[j].data.title[0]!=NULL)//修改操作记录
			j++;
			strcpy(operation[j].data.title,name);
			operation[j].status='+';
			return true;                      //修改成功
		}
	}
	return false;                             //修改失败
}
//受控文件移除目录（+&0->-）
bool del(FileAndStatus fData[],int total,char *name,FileAndStatus operation[])
{
	int i,j=0;                                 //循环因子
	for(i=0;i<=total-1;i++)
	{
		if(strcmp(fData[i].data.title,name)==0&&((fData[i].status=='0')||(fData[i].status=='+')||(fData[i].status=='M')))
		{
			fData[i].status='-';              //修改状态
		    while(operation[j].data.title[0]!=NULL&&strcmp(operation[j].data.title,name)!=0)//修改操作记录
			j++;
			strcpy(operation[j].data.title,name);
			operation[j].status='-';
		    return true;                       //修改成功
		}          
	}
	return false;                              //修改失败
}\

//回滚至某一版本，或默认最新
int update(Version_Info TotalVersion[],int To_version,int total,FileAndStatus fData[])
{
	int i=0;
	char szPath[MAX_PATH];   //szPath[]表示当前目录路径
    char szPath_Cur[MAX_PATH],szPath_Des[MAX_PATH],szPath_Del[MAX_PATH];  //szPath_Cur[],szPath_Des[]分别表示复制的源文件和目标文件的路径
	char title_temp[50];     //用于复制文件的过程中存放文件的重命名
    //判断是否有修改、丢失、删除、添加的文件
	i=0;
	//先删除文件夹中所有txt文件
    GetCurrentDirectory(MAX_PATH,szPath);
    while (i<=total-1)
	{
	   strcpy(szPath_Del,szPath);
	   if(fData[i].status!='?')
       DeleteFile(strcat(strcat(szPath_Del,"\\"),fData[i].data.title));
	   i++;
	}
    //从合适的版本中读取文件信息，复制到文件夹，并改名
    i=0;
    while (TotalVersion[To_version-1].data[i].time!=0)
	{  
		strcpy(szPath_Cur,szPath);
		strcpy(szPath_Des,szPath);
		CopyFile(strcat(strcat(szPath_Cur,"\\_svn_\\data\\"),ltoa(TotalVersion[To_version-1].data[i].time,title_temp,10)),strcat(strcat(szPath_Des,"\\"),TotalVersion[To_version-1].data[i].title),false);
		i++;													
	}
    //更改当前版本号
    return To_version;//要update的版本号	
}
//执行
bool commit(FileAndStatus fData[],Version_Info TotalVersion[],int version,char alog[],int numversion)//alog表示用户输入的log
{
	int i=0,v=0,j=0;
	v=numversion;
	bool IsModificated=false; //如果所有文件都未作修改，则返回no modification；否则返回版本号
	//判断是否有文件被修改
	while (!IsModificated && fData[i].data.time!=0)
	{
		if (fData[i].status=='+' || fData[i].status=='M'||fData[i].status=='-'||fData[i].status=='G')  
			IsModificated=true;
		i++;
	}
    TotalVersion[v].version=v+1;//如果顺序表是从0开始存的话
    TotalVersion[v].lastversion=version; //将当前版本号赋值过去
    i=0;
	char szPath_Cur[MAX_PATH];
    while (fData[i].data.time!=0 && IsModificated)
	{
		GetCurrentDirectory(MAX_PATH,szPath_Cur);
		switch (fData[i].status)
		{
	      case '0'://没修改
			  //将文件名和时间长度存入内存数组中
			  TotalVersion[v].data[j].time =fData[i].data.time;
			  strcpy(TotalVersion[v].data[j].title,fData[i].data.title);
			  j++;
			  break;
          case '+':
		  case 'M':
              //将文件名和时间长度存入内存数组中
			  TotalVersion[v].data[j].time =fData[i].data.time;
			  strcpy(TotalVersion[v].data[j].title,fData[i].data.title);
			  j++;
			  //将文件复制到_svn_文件夹中
			  char szPath_Des[MAX_PATH]; //szPath_Cur表示源文件路径，szPath_Des表示目标路径地址
			  char title_temp[50];//用于复制文件的过程中存放文件的重命名
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
		  //case 'G':; //不需要操作
	   }//endswitch
		i++;
	}//endwhile
	if (IsModificated)
	{
		strcpy(TotalVersion[v].log,alog);//记录日志log
		return true;
	}
	else
	{
		cout<<"no modification"<<endl;//当文件没修改时，输出"no modification"
		return false;
	}
}
//撤销命令
void revert(FileAndStatus fData[],int total,FileAndStatus backup[])
{
	
	int i;                                                //循环因子
	char szPath_Cur[MAX_PATH],szPath_Des[MAX_PATH];       //记录地址   
	char title_temp[50];                                  //临时储存title和time
	long time_temp;
	for(i=0;i<=total-1;i++)
	{
		GetCurrentDirectory(MAX_PATH,szPath_Cur);        
		switch(fData[i].status)
		{
		case '+':
			//修改标志
			fData[i].status='?';break; 
		case 'G':
			//将源文件复制回来
			fData[i].status='0';
			strcpy(szPath_Des,szPath_Cur);
			CopyFile(strcat(strcat(szPath_Des,"\\_svn_\\data\\"),ltoa(fData[i].data.time,title_temp,10)),strcat(strcat(szPath_Cur,"\\"),fData[i].data.title),false);
			break;
		case '-':
		case 'M':
			//复制源文件将现文件覆盖
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
//显示日志
struct qstype             //log函数栈结构体
{
	int VersionNum[MaxFileNum];
	int top;
};
void initiateqs(qstype *s)//log栈初始化
{
	s->top=-1;
}
int push(qstype *s,int x) //log入栈
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
int pop(qstype *s)        //log出栈
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
		if(!((i==0&&argument[i]=='"')||(argument[i+1]=='\0'&&argument[i]=='"')))   //如果头尾碰到引号则不输出
		{
			cout<<argument[i];
		}
	}
		cout<<endl;
}
void log(Version_Info TotalVersion[],int *version)//给出version参数时，调用该log函数
{
	cout<<"version: "<<*version<<endl;
	cout<<"log: ";
	output(TotalVersion[*version-1].log);
}
void log(Version_Info TotalVersion[],int nowversion)//没给version参数时，调用该log函数
{
	int n,version_temp=nowversion;                  //将当前版本号赋值给version_temp
	qstype *s;
	s=new qstype;
	initiateqs(s);
    while (nowversion!=0)
	{ 
		push(s,nowversion);                          //版本号入栈
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
//显示详细版本属性信息
void attribute(Version_Info TotalVersion[],int *version)
{
	cout<<"version:"<<*version<<endl;
	cout<<"file:"<<endl;
	for(int i=0;TotalVersion[*version-1].data[i].title[0]!=NULL;i++)
	cout<<TotalVersion[*version-1].data[i].title<<endl;
	cout<<"log:";
	output(TotalVersion[*version-1].log);                     //控制输出
}
//输入指令及参数命令行界面
void InputOrder(char order[],char argument[])
{
	order[0]='\0';
	argument[0]='\0';
	cout<<"svn>";
	for(int i=0,j=0;1;) 
	{ 
		char temp;                //临时的字符记录      
		temp=getch();             //接收字符  
		if(temp==13){break;}      //判断输入的是否回车	
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
			putchar(temp);         //输出临时的字符记录
		}
		else if(temp!=8)
		{
			order[i]=temp;
			putchar(temp);         //输出临时的字符记录
			i++;
		}
	}
	order[i]='\0';
	if(j!=0) argument[j-1]='\0';
	cout<<endl;
}

//将字符型参数转换成整型版本号
int ChangeIntoNum(char argument[])
{
	int argunum=0;//版本号参数为0，则为默认
	for(int i=0;argument[i]!='\0';i++)
	{
		if(argument[0]!=48&&argument[i]>=48&&argument[i]<=57)
		{
			argunum=argunum*10+(argument[i]-48);
		}
		else{return (-1);}//出错（无法转换）
	}
	return argunum;
}
//新版本保存
void save(Version_Info TotalVersion[],int *VersionNum)
{
	ofstream outfile("_svn_\\config.dat",ios::binary);
	//保存记录计数
	outfile.write((char*)&(*VersionNum),sizeof(*VersionNum));
	//保存记录
	for(int i=1;i<=*VersionNum;i++)
		outfile.write((char*)&(TotalVersion[i-1]),sizeof(TotalVersion[i-1]));
	outfile.close();
}
//初始化版本信息
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
//初始化操作信息
void operation_initialization(FileAndStatus temp[])
{
		for(int i=0;i<=49;i++)
		temp[i].data.title[0]=NULL;
}
int main()
{
	char order[15];                    //命令输入
	char argument[50];                 //命令参数
	int argunum;                       //命令参数，版本号
	bool IsControlled;                 //该文件夹是否被控制
	//当前文件夹相关信息
	int TotalFile;                     //记录当前文件夹文件总数
	FileAndStatus fData[MaxFileNum];   //文件夹中的相关文件及对应状态
	//版本信息
	Version_Info TotalVersion[MaxVersionNum];//所有版本的信息
	FileAndStatus CurrentVersion[MaxFileNum];//当前版本的信息
	FileAndStatus OperationData[MaxFileNum]; //记录操作的信息
	int NumVersion=0;                        //版本总数
	int version=0;                           //当前版本
	initialization(TotalVersion);
	operation_initialization(OperationData);
	//判断当前文件夹是否被控制
	IsControlled=ControlledOrNot();
	//受保护则加载文件
	if(IsControlled)
	{
		//加载所有版本的信息
		NumVersion=Load(TotalVersion);
		version=NumVersion;
		//加载当前最新版本的信息
		LoadVersion(TotalVersion,version,CurrentVersion);
	}
	while(1)
	{
		//输入指令及参数命令行界面
		InputOrder(order,argument);
		argunum=ChangeIntoNum(argument);
		//指令判别及部分指令的执行	
		if(!strcmp(order,"create")&&argument[0]=='\0')             //create创建受保护标志文件夹
		{
			if(!IsControlled)
			{
				IsControlled=create();
				//加载初始版本的信息
				NumVersion=Load(TotalVersion);
				//加载当前最新版本的信息
				LoadVersion(TotalVersion,version,CurrentVersion);
			}
			else{cout<<"already create!"<<endl;}
		}
		else if(!strcmp(order,"quit")&&argument[0]=='\0')          //quit退出操作
		{exit(EXIT_SUCCESS);}        
		else if(!IsControlled)                                     //确保文件夹被控制
		{cout<<"please,create first!"<<endl;}
		else if(!strcmp(order,"status")&&argument[0]=='\0')        //status显示文件比较状态
		{
		     	//扫描文件同时进行比对
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);
				show_status(fData,TotalFile);
		}
		else if(!strcmp(order,"add"))                              //add argument,添加受管理文件
		{
			TotalFile=ScanFile(fData,CurrentVersion,OperationData);
			if(!add(fData,TotalFile,argument,OperationData))
			{
				cout<<"Add Error!"<<endl;
			}
		}
		else if(!strcmp(order,"delete"))                           //delete argument,删除文件管理
		{		//扫描文件同时进行比对
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);
				if(!del(fData,TotalFile,argument,OperationData))
			{
				cout<<"Delete Error!"<<endl;
			}
		}
		else if(!strcmp(order,"update")&&argunum!=-1)               //update argunum,更新至某一版本
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
		else if(!strcmp(order,"commit"))                               //commit argument,提交任务并执行操作，以产生新版本
		{
			TotalFile=ScanFile(fData,CurrentVersion,OperationData);
			if(commit(fData,TotalVersion,version,argument,NumVersion))
			{
				NumVersion++;
				version=NumVersion;
				cout<<"version "<<version<<endl;
				LoadVersion(TotalVersion,version,CurrentVersion);	   //加载当前最新版本的信息
				save(TotalVersion,&NumVersion);                        //保存版本至配置文件
				operation_initialization(OperationData);
				TotalFile=ScanFile(fData,CurrentVersion,OperationData);//扫描文件同时进行比对
			}			
		}
		else if(!strcmp(order,"revert")&&argument[0]=='\0')            //revert,撤销操作
		{
            TotalFile=ScanFile(fData,CurrentVersion,OperationData); 
			revert(fData,TotalFile,CurrentVersion);
		    operation_initialization(OperationData);
		}
		else if(!strcmp(order,"log")&&argunum!=-1)                  //log argunum,显示版本产生日志
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
		else if(!strcmp(order,"attribute")&&argunum!=-1)            //attribute argunum,显示版本文件属性
		{
			if(argunum==0) argunum=version;
			attribute(TotalVersion,&argunum);
		}
		else{cout<<"Warning!No such orders."<<endl;}                //错误指令警告
		cout<<endl;
	}
	return 0;
}
