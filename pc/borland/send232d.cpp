//------------------------------------------------------------------------------------------------------------- 
// & - E para bits 
// ^ - OU para bits 
//------------------------------------------------------------------------------------------------------------- 
//------------------------------------------
// PROTOCOLO DE COMUNICACAO:
//------------------------------------------
// <-    DDDD - INICIA COMUNICACAO
//    -> DDDD - COMUNICACAO ESTABELECIDA
// <- -> DDdd - RECEBE OU ENVIA DADO (dd)
//    -> EE00 - DADO RECEBIDO COM SUCESSO
//    -> EE01 - ERRO NA RECEPCAO DO DADO
// <-    EEDD - ENCERRA COMUNICACAO
//------------------------------------------
#include <stdio.h> 
#include <conio.h> 
#include <dos.h> 
#include <string.h> 
#include <windows.h>  //Necessário para: LoadLibrary(), GetProcAddress() e HINSTANCE.

unsigned char endlsb, endlsb_bkp;
unsigned char endmsb, endmsb_bkp;
unsigned char RetByte;       // Para armazenar o valor recebido da Porta Paralela.
unsigned char RetInput[2]; 
HANDLE hSerial = NULL;

void delay(unsigned int ttempo);
int IniciaSerial();
void TerminaSerial();
DWORD EnviaComando(unsigned char sbyte);
DWORD EnviaCtrl(unsigned char sbyte); 
DWORD RecebeComando();

// Programa Principal
int main(int argc, char *argv[])
{
  FILE *fp;
	char xfilebin[100] = "D:\\Projetos\\RaspberryPi\\MMSJOS\\";
  char xfilename[16] = "";
	int vnumFF = 0;
	int verro, cc, ee;
	int vnumerros;
	unsigned char dados;
	unsigned char dadosrec;

	if (argc < 2)
	{
		printf("Erro. Nao foi passado o programa a ser enviado.\n");
		printf("Sintaxe: modulo <nome do arquivo>\n");
		return 0;
	}
	
	// Variaveis de Saida

	clrscr();

	// Inicio da Rotina
	printf(">Iniciando Envio de Arquivo via porta Serial. Usando %s.\n", argv[1]);
	// Enviando Dados (Adress LSB -> MSB -> Dados)
	endlsb = 0x00;
	endmsb = 0x00;

	printf(">Inicializando Porta Serial.\n");
	IniciaSerial();

	printf(">Abrindo Comunicacao.\n");
	EnviaComando(0xDD);
	
	printf(">Recebendo Confirmacao.\n");

  RetInput[0] = 0x00;

  while (RetInput[0] != 0xDD)
	  RecebeComando();

  RecebeComando();
	
	if (RetInput[0] != 0xDD)
		vnumFF = 255;

	// Gravar Dados
	if (vnumFF == 0) {
		printf(">Abrindo Arquivo.\n");
  	strcat(xfilebin,argv[1]);
  	strcat(xfilename,argv[1]);
  	fp = fopen(xfilebin,"rb");
    
    if (fp == NULL) {
  		printf(">Erro ao Abrir Arquivo %s.\n", xfilebin);
      vnumFF = 255;
    }
  }
	else
		printf(">Comunicacao nao Estabelecida. Recebido %02X:%02X. Verifique.\n", RetInput[0], RetInput[1]);

  if (vnumFF != 255) {
		printf(">Enviando Nome do Arquivo.\n");
    
    ee = 1;		
    for (cc = 0; cc <= 15; cc++) {
      if (xfilename[cc] == '\0')
        ee = 0;
        
      if (ee)
        dados = xfilename[cc];
      else
        dados = 0x00;
        
  		verro = 1;
  		while (verro)
  		{
  			printf("--> : %02X",dados);
  
  			EnviaComando(dados);

        RetInput[0] = 0x00;

        while (RetInput[0] != 0xDD)
    			RecebeComando();

        RecebeComando();
  			
  			if (RetInput[0] != dados) {
  				printf(" --> Erro Leitura.\n");
    			EnviaCtrl(0x01); // Comparação Byte Com Erro: 0xEE + 0x01
        }
  			else {
  				printf(" --> OK.\r");
     			EnviaCtrl(0x00); // Comparação Byte OK: 0xEE + 0x00
          verro = 0;
        }        
      }
    }
    
    printf("\n>Enviando Dados.\n");

  	while ((!feof(fp)) && (vnumFF <= 20))
  	{
  		dados = getc(fp);
  		
  		if (dados == 0xFF)
  			vnumFF += 1;
  
  		if (dados != 0xFF)
  			vnumFF = 0;
  		
  		verro = 1;
  		while (verro)
  		{
  			// Grava Dados
  			printf(">%02X",endmsb);
  			printf("%02X",endlsb);
  			printf(" : %02X",dados);
  
  			// Grava Dados
  			EnviaComando(dados);

        RetInput[0] = 0x00;

        while (RetInput[0] != 0xDD)
    			RecebeComando();

        RecebeComando();
  			
  			if (RetInput[0] != dados) {
  				printf(" --> Erro Leitura.\n");
    			EnviaCtrl(0x01); // Comparação Byte Com Erro: 0xEE + 0x01
        }
  			else {
  				printf(" --> OK.\r");
     			EnviaCtrl(0x00); // Comparação Byte OK: 0xEE + 0x00
          verro = 0;
        }        
  		}
  		
  		if (endlsb == 0xFF)
  		{
  			endmsb += 1;
  			endlsb = 0x00;
  		}
  		else
  			endlsb += 1;
  	}
  
  	fclose(fp);
  
  	printf("\n>Finalizando Comunicacao.\n");
  
  	EnviaCtrl(0xDD);
  	TerminaSerial();
  
  	printf(">Dados Enviados.\n");
  }
    
	return(0);
}

void delay(unsigned int ttempo)
{
	for (unsigned int i = 0; i <= ttempo; i++);
}

int IniciaSerial(){ 
	char *NomePorta = "COM3"; //COM1, COM2... 

	hSerial = CreateFile(NomePorta,	 //Nome da porta. 
 						 GENERIC_READ|GENERIC_WRITE, //Para leitura e escrita. 
						 0,	 //(Zero) Nenhuma outra abertura será permitida. 
						 NULL,	 //Atributos de segurança. (NULL) padrão. 
						 OPEN_EXISTING,	 //Criação ou abertura. 
						 0,	 //Entrada e saída sem overlapped. 
						 NULL	 //Atributos e Flags. Deve ser NULL para COM. 
						 ); 

	if(hSerial == INVALID_HANDLE_VALUE) 
		return false; //Erro ao tentar abrir a porta 

	DCB dcb; //Estrutura DCB é utilizada para definir todos os 
		 	 // parâmetros da comunicação. 

	if( !GetCommState(hSerial, &dcb)) 
		return false; //// Erro na leitura de DCB. 

	dcb.BaudRate = CBR_19200; 
	dcb.ByteSize = 8; 
	dcb.Parity = NOPARITY; 
	dcb.StopBits = ONESTOPBIT; 
  dcb.fDtrControl = 0x00; // DISABLE DTR
  dcb.fRtsControl = 0x00; // DISABLE RTS
  
	//Define novo estado. 
	if( SetCommState(hSerial, &dcb) == 0 ) 
		return false; //Erro. 

	return true;
} 

// fecha porta serial 
void TerminaSerial(){ 
	CloseHandle( hSerial );	 //Fecha a porta 
} 

// monta comando para ser enviado 
DWORD EnviaComando(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xDD;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

DWORD EnviaCtrl(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xEE;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

DWORD RecebeComando(){ 
	DWORD BytesLidos = 0; 

	ReadFile( hSerial, RetInput, 1, &BytesLidos, NULL ); 

	return BytesLidos;
} 
