/*
  ============================================================
                    TELEMETRIA 2026
                      MODULO SD CARDd
  ============================================================
*/

#include <SPI.h>
#include <SD.h>


// ============================================================
// CONFIGURACAO
// ============================================================

// Pino CS do cartao SD
const int SD_CS = 10;

// Nome do arquivo CSV
const char* arquivoSD = "telemetria.csv";


// ============================================================
// INICIALIZAR SD
// ============================================================

bool iniciarSD() {

  if (!SD.begin(SD_CS)) {

    Serial.println("[SD] Erro ao inicializar!");

    return false;
  }

  Serial.println("[SD] Inicializado!");

  return true;
}


// ============================================================
// CRIAR ARQUIVO CSV
// ============================================================

bool criarArquivoSD() {

  // Se o arquivo ja existir, nao cria novamente
  if (SD.exists(arquivoSD)) {
    return true;
  }

  File arquivo = SD.open(arquivoSD, FILE_WRITE);

  if (!arquivo) {

    Serial.println("[SD] Erro ao criar arquivo!");

    return false;
  }

  // Cabecalho do CSV
  arquivo.println(
    "tempo,tensao,corrente,temperatura"
  );

  arquivo.close();

  return true;
}


// ============================================================
// SALVAR DADOS
// ============================================================

bool salvarDadosSD(
  unsigned long tempo,
  float tensao,
  float corrente,
  float temperatura
) {

  File arquivo = SD.open(
    arquivoSD,
    FILE_WRITE
  );

  if (!arquivo) {

    Serial.println("[SD] Erro ao abrir arquivo!");

    return false;
  }

  arquivo.print(tempo);
  arquivo.print(",");

  arquivo.print(tensao, 2);
  arquivo.print(",");

  arquivo.print(corrente, 2);
  arquivo.print(",");

  arquivo.println(temperatura, 2);

  arquivo.close();

  return true;
}


  Serial.println("[SD] Cartao inicializado com sucesso!");

  return true;
}


// ============================================================
// CRIAR ARQUIVO CSV
// ============================================================

bool criarArquivoSD() {

  // Verifica se o arquivo ja existe
  if (SD.exists(arquivoSD)) {

    Serial.println("[SD] Arquivo CSV ja existe.");

    return true;
  }


  Serial.println("[SD] Criando arquivo CSV...");


  // Cria o arquivo
  File arquivo = SD.open(
    arquivoSD,
    FILE_WRITE
  );


  // Verifica se conseguiu criar
  if (!arquivo) {

    Serial.println(
      "[SD] ERRO: nao foi possivel criar o arquivo."
    );

    return false;
  }


  // ----------------------------------------------------------
  // CABECALHO DO CSV
  // ----------------------------------------------------------

  arquivo.println(
    "tempo,tensao,corrente,temperatura"
  );


  // Fecha o arquivo
  arquivo.close();


  Serial.println("[SD] Arquivo criado com sucesso!");

  return true;
}


// ============================================================
// SALVAR DADOS NO CSV
// ============================================================

bool salvarDadosSD(
  unsigned long tempo,
  float tensao,
  float corrente,
  float temperatura
) {


  // Abre o arquivo para adicionar uma nova linha
  File arquivo = SD.open(
    arquivoSD,
    FILE_WRITE
  );


  // Verifica se o arquivo abriu corretamente
  if (!arquivo) {

    Serial.println(
      "[SD] ERRO: nao foi possivel abrir o arquivo."
    );

    return false;
  }


  // ----------------------------------------------------------
  // ESCREVER DADOS
  // ----------------------------------------------------------

  arquivo.print(tempo);
  arquivo.print(",");

  arquivo.print(tensao, 2);
  arquivo.print(",");

  arquivo.print(corrente, 2);
  arquivo.print(",");

  arquivo.println(temperatura, 2);


  // Fecha o arquivo
  arquivo.close();


  return true;
}