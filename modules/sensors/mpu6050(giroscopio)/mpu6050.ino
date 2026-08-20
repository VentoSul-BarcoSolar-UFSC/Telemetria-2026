/*
  ============================================================
                    MÓDULO MPU6050 - TELEMETRIA
  ============================================================

  Responsável por:

  - Comunicação I2C com o MPU6050;
  - Leitura do acelerômetro;
  - Leitura do giroscópio;
  - Calibração do giroscópio;
  - Conversão dos valores brutos;
  - Cálculo de Pitch;
  - Cálculo de Roll;
  - Cálculo de Yaw;
  - Aplicação de filtro complementar.

  Este módulo funciona de forma independente.

  Ele NÃO possui dependência com:

  - SD Card;
  - LCD;
  - Outros sensores;
  - Sistema principal de telemetria.

  Os dados gerados poderão posteriormente ser utilizados pelo
  código principal e enviados para outros módulos.

  ============================================================
*/

#include <Wire.h>
#include <math.h>


// ============================================================
// CONFIGURAÇÃO DO MPU6050
// ============================================================

// Endereço I2C padrão do MPU6050
const int MPU = 0x68;


// ============================================================
// ESCALAS DO SENSOR
// ============================================================

/*
  Acelerômetro configurado para ±2g.

  O MPU6050 fornece valores brutos que precisam ser divididos
  por 16384 para obter o valor em "g".
*/

const float ACCEL_SCALE = 16384.0;


/*
  Giroscópio configurado para ±250 graus por segundo.

  Os valores brutos são divididos por 131 para obter
  a velocidade angular em °/s.
*/

const float GYRO_SCALE = 131.0;


// ============================================================
// FILTRO COMPLEMENTAR
// ============================================================

/*
  O filtro complementar combina:

  - Giroscópio:
      Responde rapidamente aos movimentos.

  - Acelerômetro:
      Possui uma referência estável em relação à gravidade.

  ALPHA define quanto o sistema confia no giroscópio.

  Quanto maior:
      Mais resposta rápida.

  Quanto menor:
      Mais estabilidade do acelerômetro.
*/

const float ALPHA = 0.96;


// ============================================================
// VARIÁVEIS DE ORIENTAÇÃO
// ============================================================

// Ângulos calculados pelo sistema
float pitch = 0;
float roll = 0;
float yaw = 0;


// ============================================================
// CALIBRAÇÃO DO GIROSCÓPIO
// ============================================================

/*
  O giroscópio pode apresentar pequenos valores mesmo quando
  o sensor está completamente parado.

  Essas variáveis armazenam o erro médio de cada eixo.

  Durante a leitura, esse valor será removido dos dados.
*/

float gyroBiasX = 0;
float gyroBiasY = 0;
float gyroBiasZ = 0;


// ============================================================
// CONTROLE DE TEMPO
// ============================================================

// Armazena o tempo da última leitura
unsigned long tempoAnterior = 0;


// ============================================================
// FREQUÊNCIA DO LOOP
// ============================================================

/*
  60 Hz = aproximadamente 16,67 ms por ciclo.

  1 segundo = 1.000.000 microssegundos

  1.000.000 / 60 ≈ 16667
*/

const unsigned long INTERVALO_LOOP_US = 16667UL;

unsigned long tempoLoopAnterior = 0;


// ============================================================
// FUNÇÃO: CALIBRAR GIROSCÓPIO
// ============================================================

void calibrarGiroscopio() {

  Serial.println("Calibrando giroscopio...");
  Serial.println("Mantenha o sensor parado.");

  const int N = 200;

  long somaX = 0;
  long somaY = 0;
  long somaZ = 0;


  for (int i = 0; i < N; i++) {

    // Solicita dados do giroscópio
    Wire.beginTransmission(MPU);

    // Registrador inicial do giroscópio
    Wire.write(0x43);

    Wire.endTransmission(false);

    // Solicita 6 bytes:
    // GX High
    // GX Low
    // GY High
    // GY Low
    // GZ High
    // GZ Low

    Wire.requestFrom(MPU, 6, true);


    if (Wire.available() == 6) {

      int16_t gx = Wire.read() << 8 | Wire.read();
      int16_t gy = Wire.read() << 8 | Wire.read();
      int16_t gz = Wire.read() << 8 | Wire.read();

      somaX += gx;
      somaY += gy;
      somaZ += gz;
    }

    delay(5);
  }


  // Calcula o valor médio do erro

  gyroBiasX = (somaX / (float)N) / GYRO_SCALE;

  gyroBiasY = (somaY / (float)N) / GYRO_SCALE;

  gyroBiasZ = (somaZ / (float)N) / GYRO_SCALE;


  Serial.println("Calibracao concluida.");

}


// ============================================================
// SETUP
// ============================================================

void setup() {

  // Comunicação serial
  Serial.begin(115200);


  // Inicializa comunicação I2C
  Wire.begin();


  // ==========================================================
  // INICIALIZAÇÃO DO MPU6050
  // ==========================================================

  /*
    O registrador 0x6B controla o modo de energia.

    Escrever 0 faz o MPU6050 sair do modo sleep.
  */

  Wire.beginTransmission(MPU);

  Wire.write(0x6B);

  Wire.write(0);

  Wire.endTransmission(true);


  delay(100);


  // ==========================================================
  // CALIBRAÇÃO
  // ==========================================================

  calibrarGiroscopio();


  // Inicializa os temporizadores

  tempoAnterior = millis();

  tempoLoopAnterior = micros();


  // ==========================================================
  // CABEÇALHO CSV
  // ==========================================================

  /*
    Esses dados podem ser utilizados posteriormente
    por programas de monitoramento e gráficos.
  */

  Serial.println(
    "tempo_ms,"
    "ax,ay,az,"
    "gx,gy,gz,"
    "pitch,roll,yaw"
  );

}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop() {

  // ==========================================================
  // VARIÁVEIS BRUTAS DO SENSOR
  // ==========================================================

  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;

  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;


  // ==========================================================
  // LEITURA DO ACELERÔMETRO
  // ==========================================================

  /*
    Os dados do acelerômetro começam no registrador:

    0x3B

    São lidos 6 bytes:

    AX -> 2 bytes
    AY -> 2 bytes
    AZ -> 2 bytes
  */

  Wire.beginTransmission(MPU);

  Wire.write(0x3B);

  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 6, true);


  if (Wire.available() < 6) {

    return;

  }


  AcX = Wire.read() << 8 | Wire.read();

  AcY = Wire.read() << 8 | Wire.read();

  AcZ = Wire.read() << 8 | Wire.read();


  // ==========================================================
  // LEITURA DO GIROSCÓPIO
  // ==========================================================

  /*
    Os dados do giroscópio começam no registrador:

    0x43

    São lidos:

    GX
    GY
    GZ
  */

  Wire.beginTransmission(MPU);

  Wire.write(0x43);

  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 6, true);


  if (Wire.available() < 6) {

    return;

  }


  GyX = Wire.read() << 8 | Wire.read();

  GyY = Wire.read() << 8 | Wire.read();

  GyZ = Wire.read() << 8 | Wire.read();


  // ==========================================================
  // CONVERSÃO DO ACELERÔMETRO
  // ==========================================================

  /*
    Converte os valores brutos para "g".
  */

  float ax = AcX / ACCEL_SCALE;

  float ay = AcY / ACCEL_SCALE;

  float az = AcZ / ACCEL_SCALE;


  // ==========================================================
  // CONVERSÃO DO GIROSCÓPIO
  // ==========================================================

  /*
    Converte os valores para °/s.

    O valor da calibração também é removido.
  */

  float gx = (GyX / GYRO_SCALE) - gyroBiasX;

  float gy = (GyY / GYRO_SCALE) - gyroBiasY;

  float gz = (GyZ / GYRO_SCALE) - gyroBiasZ;


  // ==========================================================
  // CÁLCULO DO TEMPO ENTRE LEITURAS
  // ==========================================================

  unsigned long agora = millis();

  float dt = (agora - tempoAnterior) / 1000.0;

  tempoAnterior = agora;


  // ==========================================================
  // ÂNGULOS CALCULADOS PELO ACELERÔMETRO
  // ==========================================================

  /*
    O acelerômetro pode ser utilizado para calcular
    a inclinação em relação à gravidade.

    Pitch -> inclinação frontal/traseira

    Roll -> inclinação lateral
  */

  float pitchAcc =
    atan2(
      -ax,
      sqrt(ay * ay + az * az)
    ) * 180.0 / PI;


  float rollAcc =
    atan2(
      ay,
      az
    ) * 180.0 / PI;


  // ==========================================================
  // FILTRO COMPLEMENTAR
  // ==========================================================

  /*
    Combina:

    Ângulo anterior + movimento do giroscópio

    com

    Ângulo calculado pelo acelerômetro.
  */

  pitch =
    ALPHA *
    (pitch + gx * dt)

    +

    (1 - ALPHA) *
    pitchAcc;


  roll =
    ALPHA *
    (roll + gy * dt)

    +

    (1 - ALPHA) *
    rollAcc;


  // ==========================================================
  // CÁLCULO DO YAW
  // ==========================================================

  /*
    O MPU6050 não possui magnetômetro.

    Portanto, não existe uma referência absoluta
    para o eixo Z.

    O Yaw é calculado apenas integrando
    a velocidade angular.

    Com o tempo, pode ocorrer deriva.
  */

  yaw += gz * dt;


  // ==========================================================
  // ENVIO DOS DADOS VIA SERIAL
  // ==========================================================

  Serial.print(agora);
  Serial.print(",");

  Serial.print(ax, 4);
  Serial.print(",");

  Serial.print(ay, 4);
  Serial.print(",");

  Serial.print(az, 4);
  Serial.print(",");

  Serial.print(gx, 4);
  Serial.print(",");

  Serial.print(gy, 4);
  Serial.print(",");

  Serial.print(gz, 4);
  Serial.print(",");

  Serial.print(pitch, 4);
  Serial.print(",");

  Serial.print(roll, 4);
  Serial.print(",");

  Serial.println(yaw, 4);


  // ==========================================================
  // CONTROLE DE FREQUÊNCIA
  // ==========================================================

  /*
    Mantém aproximadamente 60 ciclos por segundo.
  */

  unsigned long tempoGastoUs =
    micros() - tempoLoopAnterior;


  if (tempoGastoUs < INTERVALO_LOOP_US) {

    delayMicroseconds(
      INTERVALO_LOOP_US - tempoGastoUs
    );

  }


  tempoLoopAnterior = micros();

}