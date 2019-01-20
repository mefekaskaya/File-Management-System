#include <fms.h>
#include <string.h>
#include <json/json.h>

int indexFileChanged = 0;

int fileSize(FILE*f){//dosya boyutu veren basit bi fonksiyon
	int t = ftell(f);
	fseek(f,0,SEEK_END);
	int size = ftell(f);
	fseek(f,t,SEEK_SET);
	return size;
}

FILE* indexFile;

int recordSize = -1;//eğer -1den farklı bi deðer deðilse düzgün atanmamýþtýr yani düzgün çalýþmamýþtýr program
int keyStart = -1;//bazý fonksiyonlarda kontrol için
int keyEnd = -1;
int keyLength = -1;
int order = -1;
int indexCount = -1;//kaç tane index record olduðunu göstermekte

typedef struct _indexer{
	int filePointer;
	char key[20];//20 belirledim max key length olarak , eðer 10luk gelirse yine çalýþýr.
} indexer;

typedef struct _fileheader{
	int recordLength;
	int keyStart;
	int keyEnd;
	int order;//1-> ASC, 2->DESC
	int open;
} fileheader;

char dataName[30];
char indexName[30];

fileheader fh;//açýk olan dosya için kullandýðým fileheader

void setFileNames(char*fileName){

	memset(dataName,0,30);
	memset(indexName,0,30);

	sprintf(dataName,"%s.dat",fileName);
	sprintf(indexName,"%s.ndx",fileName);
}

int myFileCreate(char * myfilename, char *jsonfilename){

	setFileNames(myfilename);

	indexFile = fopen(indexName,"w+");
	fclose(indexFile);

	FILE*jsonFile = fopen(jsonfilename,"r+");
	int jsonsize = fileSize(jsonFile);
	//json dosyasını içeriğini okuyup parsera verdim
	char*jsontext = malloc(jsonsize +1);
        memset(jsontext,0,jsonsize+1);
	
	fseek(jsonFile,0,SEEK_SET);
	fread(jsontext,jsonsize,1,jsonFile);
	fclose(jsonFile);
	printf("json file\n%s\n",jsontext);

	json_object* jobj = json_tokener_parse(jsontext);

	if(jobj == NULL){
		printf("Error: json parser\n");
		return 0;
	}
	//json objecsindeki keylere göre değerleri aldım
	json_object_object_foreach(jobj,key,val){
		
		if(strcmp("recordLength",key) == 0){
			recordSize = json_object_get_int(val);
		}
		else if(strcmp("keyStart",key) == 0){
			keyStart = json_object_get_int(val);
		}
		else if(strcmp("keyEnd",key) == 0){
			keyEnd = json_object_get_int(val);
		}
		else if(strcmp("order",key) == 0){
			const char*ostr = json_object_get_string(val);
			if(strcmp(ostr,"ASC") == 0){
				order = 1;
			}
		}
	}
	//fhnin değerlerini atadım dosyanınbaşına yazmak için
	fh.keyStart = keyStart;
	fh.keyEnd = keyEnd;
	fh.recordLength = recordSize;
	fh.order = order;
	fh.open = 1;
	
	//data dosyasını açıp header değerini yazdırdım
	p = fopen(dataName,"w+");
	if(p == NULL){
		printf("Error: data file open\n");
		return 0;
	}
	
	fwrite(&fh,1,sizeof(fh),p);
	
	indexCount = 0;
	int indexRecordLength = sizeof(indexer);
	ndx = malloc(100 * indexRecordLength);//100 tane indexlik yer aldým eðer daha fazlasý gerekirse sayý artýrýlabilir
		
	return 1;
}

int myFileOpen(char * myfilename){

	setFileNames(myfilename);

	p = fopen(dataName,"r+");
	if(p == NULL){
		printf("Error: data file open\n");
		return 0;
	}

	indexFile = fopen(indexName,"r+");
	if(p == NULL){
		printf("Error: index file open\n");
		fclose(p);
		return 0;
	}

	fread(&fh,1,sizeof(fh),p);//dosyanýn baþýndan dosya headerý kadar okuyacam

	if(fh.open == 1){
		printf("Warning: File already opened\n");
	}

	fh.open = 1;//dosyayý açtýðýmýz için içindeki deðeri 1 yapýp dosyaya geri yazýyoruz
	fseek(p,0,SEEK_SET);
	fwrite(&fh,1,sizeof(fh),p);

	keyStart = fh.keyStart;//programda kullanýlacak deðerleri dosyadaki headerdan aldým
	keyEnd = fh.keyEnd;
	recordSize = fh.recordLength;
	order = fh.order;

	keyLength = keyEnd - keyStart;//index için kullanýlan key lengthini buldum

	int fSize = fileSize(indexFile);//index dosyasýnýn boyutunu hesapladým.

	int indexRecordLength = sizeof(indexer);

	indexCount = fSize / indexRecordLength;//dosyada kaç tane record varsa o kadar yer açýlacak

	ndx = malloc(100 * indexRecordLength);//100 tane indexlik yer aldým eðer daha fazlasý gerekirse sayý artýrýlabilir

	fseek(indexFile,0,SEEK_SET);
	fread(ndx,sizeof(indexer),indexCount,indexFile);//fakat dosyadan ne kadar index varsa o kadar okuacak

	fclose(indexFile);

	return 1;
}

int myFileClose(){

	fh.open = 0;//dosyanýn baþýndaki opený false yapýp dosyaya yazýp dosyayý kapatýyorum
	fseek(p,0,SEEK_SET);
	fwrite(&fh,sizeof(fh),1,p);
	if(fclose(p) != 0){//eðer bu deðer 0dan farklý dönerse kapanamamýþtýr demektir
		printf("Error: data file close\n");
		return 0;
	}

	myFileReindex();//if neccessary re index the file

	return myWriteIndex();//gelen deðeri direk döndürdüm çünkü en son iþlem bu
}

int myFileWrite(void *data){

	indexer* indexArray = (indexer*) ndx;//void*'ý indexer aray türüne çevirdimki daha rahat iþlem yapabiliyim

	//þimdi index yeri indexCounttur o yüzden eklemek istediðimiz indexi indexer[indexCount]

	fseek(p,0,SEEK_END);
	int pointer = ftell(p);//dosyada yazýlacak yer dosyada nerde olduðunu gösterir

	indexArray[indexCount].filePointer = pointer;//bu deðeri arraydeki yerine kaydetim
	
	//verilen data verisinden keyStartdan baþlayarak keyEnd-keyStart kadar yani uzunluðu kadar
	//yer index içindeki key stringine kaydettim
	strcpy(indexArray[indexCount].key, (data + keyStart - 1));
	
	//böylece index eklenmiþ oldu fakat sýralama bozuk bunun için deðiþkeni 1e getirdim ki reindex yapýlsýn diye
	indexFileChanged = 1;
	myFileReindex();
	
	fseek(p,0,SEEK_END);
	//data dosyasýný sonuna gelen datayý yazýyorum
	fwrite(data,recordSize,1,p);
	
	indexCount++;	
	
	return 1;
}

int myFileDelete(void * key){
	//verilen keyle sadece index arrayinden silicem böylece aramalarda çýkmayacak
	//data dosyasýndan silmek çok fazla file i/o gerektirir
	//index arrayinden silmek yeterli olacaktýr

	indexer* indexArray = (indexer*) ndx;//index pointerý index arrayi haline getirdim
	char*keyStr = (char*)key;//verilen keyide string türüne çevirdim

	int i;
	for(i = 0;i< indexCount;i++){//bulunan btün indexleri tek tek gezicem
		if(strcmp(keyStr, indexArray[i].key) == 0){

			//keyý buldum demektir sondaki index kaydýný bunun üzerine yazýp index saysýný azaltarak simiþ olucam
			//hatta 1 tane record olsa bile üzerine anlamsýz deðerler yazacak fakat index sayýsý 0a indiði için bi sýkýntý olmayacak
			//yeni gelen record üzerine yazýlacak ve normla çalýþmaya devam edecek

			void*lastRecord = &indexArray[indexCount-1];//deðiþtirilecek record
			void*currenRecord = &indexArray[i];//yazýlacak yer

			memcpy(currenRecord, lastRecord, sizeof(indexer));//böylece silinecek recordun üzerine yazýlmýþ oldu
			indexCount--;//ve index saysýný azaltarak silmiþ oldum
			
			//indexi sırala çünkü silerken bozulmuştur
			myFileReindex();			
			
			return 1;//baþarýlý bir þekilde sildim
		}
	}

	return 0;//eðer herhangi bir indexe denk gelmezsem bulamamýþýmdýr
}

int myWriteIndex(){
	indexFile = fopen(indexName,"w+");//dosyayý yeniden yaratmamým nedeni eðer bir record silinirse izi kalabilir o yüzden 0dan yarattým

	if(indexFile == NULL){
		printf("Error: index file open\n");
		return 0;
	}

	int size = indexCount * sizeof(indexer);

	fwrite(ndx,size,1,indexFile);
	if(fclose(indexFile) != 0){
		printf("Error: index file close\n");
		return 0;
	}

	return 1;
}

void printIndexingFile(){
	printf("Indexing File\n");

	FILE*f = fopen(indexName,"r");
	int s = fileSize(f);

	int indexRecordLength = sizeof(indexer);

	int ic = s / indexRecordLength;

	indexer* in = malloc(s);
	fread(in,s,1,f);

	int i;
	for(i=0;i<ic;i++){
		printf("Pointer:%d Key:%s\n",in[i].filePointer,in[i].key);
	}
}

//need to implement
void printDataFile(void(*printRecord)(void*data)){
	//data dosyasýnýn yazýlabilmesi için ne tür bir struct yapýsý kullanýldýðýnýn bilinmesi gerekir
	//bize verilen veri void* türü olduðu için herhangi bir þekilde verilen yazýlmasý mümkün deðil
	//bir struct yapýsýnýn içindeki verilerin yerinin deðiþmesi bile okumanýn farklý þekilde
	//yapýlmasý gerekir. ve her seferde farklý bi veri yapýsý verilebileceði için mümkün deðildir

	//olasý çözüm bu fonksiyonun bir fonksiyon pointerý alýp dosyada bulunan her record lengthini
	//o fonksiyona teker teker göndermesi olacaktýr
	printf("Data File\n");
	
	//önce kaç tane record varsa dosyada onu bulmak lazým
	int recCount = 0;
	int fsize = fileSize(p);

	//dosya boyutundan fileheaderý çýkartýrsak saddece veriler kalýr geriye
	recCount = (fsize - sizeof(fileheader)) / recordSize;

	void*tempdata = malloc(recordSize);//fonksiyona gönderilecek yedek veri

	int i = 0;
	for(i=0;i< recCount;i++){
		int tempPointer = sizeof(fileheader) + i * recordSize;//sýradaki recordun dosyadaki yeri

		fseek(p,tempPointer,SEEK_SET);//dosyada verinin yerine gidip okudum
		fread(tempdata,1,recordSize,p);

		printRecord(tempdata);//kullanýcýnýn bu kütüphaneye kendi verisi yazdýrmasý için verdiði fonksiyonu
		//veriyle çaðýrýyoruz
	}
}

int myFileFind(void * key, void * data){

	if(data == NULL){//eðer data verisi için yer alýnmamýþsa hata vericem
		//çünkü verilen deðerde
		return 0;
	}

	indexer* indexArray = (indexer*) ndx;//index pointerý index arrayi haline getirdim
	char*keyStr = (char*)key;//verilen keyide string türüne çevirdim

	int i;
	for(i = 0;i< indexCount;i++){//bulunan btün indexleri tek tek gezicem
		if(strcmp(keyStr, indexArray[i].key) == 0){
			//
			fseek(p,indexArray[i].filePointer,SEEK_SET);//dosyada verinin olduðu yere gelip
			fread(data,recordSize,1,p);//verilen pointerýn üstüne yazarým

			return 1;//baþarýlý bir þekilde veri döndürüldü
		}
	}

	return 0;//eðer keyi bulmazsa 0 döndürür
}

int myFileFindNext(void * key, void * data){
	//find dan tek farký indexCounttan 1 eksik arýyacam çünkü eðer sonraki elemansa bakmaya gerek yok çünkü ondan sonra bi eleman yok
	//ver veriyi dosadan okurkan ondan bir sonraki eleman okunacak

	if(data == NULL){//eðer data verisi için yer alýnmamýþsa hata vericem
		//çünkü verilen deðerde
		return 0;
	}

	indexer* indexArray = (indexer*) ndx;//index pointerý index arrayi haline getirdim
	char*keyStr = (char*)key;//verilen keyide string türüne çevirdim

	int i;
	for(i = 0;i< indexCount - 1;i++){//bulunan btün indexleri tek tek gezicem
		if(strcmp(keyStr, indexArray[i].key) == 0){
			//
			fseek(p,indexArray[i+1].filePointer,SEEK_SET);//dosyada verinin bir sonrakisindeki yerden alýp okuyacam
			fread(data,recordSize,1,p);//verilen pointerýn üstüne yazarým

			return 1;//baþarýlý bir þekilde veri döndürüldü
		}
	}

	return 0;
}

int compare_func(void*d1,void*d2){
	indexer*i1 = (indexer*) d1;
	indexer*i2 = (indexer*) d2;

	if(order == 1){//eðer artan bir þekilde sýranacaksa ilk gelenin daha fazla olmasýný bekleriz
		return strcmp(i1->key,i2->key);
	}else{
		return strcmp(i2->key,i1->key);//deðerlerin yerlerini deðiþtirer deðeri - ile çarpma etkisi yaparak sýrayý tersine çevirir
	}
}

int myFileReindex(){

	qsort((void*) ndx,indexCount,sizeof(indexer),compare_func);
	//qsort fonksiyonu verilen deðerlere göre soring yapar
	//ilk deðer array, ikincisi kaç tane eleman olduðu, üçüncüsü her verinin boyutu, sonraki ise karþýlaþtýrma fonksiyonu
	//fonksiyon her iki deðeri veri boyutu kadar alýp compare fonksiyonuna gönderir gelen deðer göre artan veya azalan olacak þekilde
	//quick sort algortimasýný kullanakarak sýralar

	return 1;
}