# MPU6050 — Telemetria 2026

Módulo responsável pela leitura e processamento dos dados do sensor MPU6050.

O MPU6050 possui um acelerômetro e um giroscópio de 3 eixos, permitindo medir aceleração e velocidade angular nos eixos X, Y e Z.

Este módulo realiza a leitura dos sensores, calibra o giroscópio e utiliza um filtro complementar para calcular os ângulos de orientação.

## Bibliotecas

O módulo utiliza as seguintes bibliotecas:

```cpp
#include <Wire.h>
#include <math.h>
```

### Wire

A biblioteca `Wire` é utilizada para realizar a comunicação I2C com o MPU6050.

### math.h

A biblioteca `math.h` é utilizada para realizar cálculos matemáticos, como raiz quadrada e funções trigonométricas.

---

# Comunicação I2C

O endereço I2C padrão utilizado pelo MPU6050 é:

```cpp
const int MPU = 0x68;
```

A comunicação é inicializada utilizando:

```cpp
Wire.begin();
```

Durante a inicialização, o MPU6050 é retirado do modo de espera através do registrador `0x6B`.

```cpp
Wire.beginTransmission(MPU);
Wire.write(0x6B);
Wire.write(0);
Wire.endTransmission(true);
```

---

# Acelerômetro

O acelerômetro mede a aceleração nos três eixos:

- X
- Y
- Z

Os valores lidos pelo MPU6050 são valores brutos e precisam ser convertidos para a unidade `g`.

Neste módulo, o acelerômetro está configurado para a escala de ±2g.

```cpp
const float ACCEL_SCALE = 16384.0;
```

A conversão é realizada dividindo o valor bruto pela escala:

```cpp
float ax = AcX / ACCEL_SCALE;
float ay = AcY / ACCEL_SCALE;
float az = AcZ / ACCEL_SCALE;
```

---

# Giroscópio

O giroscópio mede a velocidade angular nos eixos:

- X
- Y
- Z

Neste módulo, o giroscópio utiliza a escala de ±250°/s.

```cpp
const float GYRO_SCALE = 131.0;
```

Os valores são convertidos para graus por segundo:

```cpp
float gx = GyX / GYRO_SCALE;
float gy = GyY / GYRO_SCALE;
float gz = GyZ / GYRO_SCALE;
```

---

# Calibração do Giroscópio

Mesmo quando o sensor está parado, o giroscópio pode apresentar pequenos valores devido a ruídos e desvios internos.

Durante a inicialização, o módulo realiza uma calibração com múltiplas leituras.

```cpp
const int N = 200;
```

Os valores são somados e posteriormente é calculada uma média para cada eixo.

```cpp
gyroBiasX = (somaX / (float)N) / GYRO_SCALE;
gyroBiasY = (somaY / (float)N) / GYRO_SCALE;
gyroBiasZ = (somaZ / (float)N) / GYRO_SCALE;
```

Esse valor médio é utilizado como correção durante as leituras.

```cpp
float gx = (GyX / GYRO_SCALE) - gyroBiasX;
float gy = (GyY / GYRO_SCALE) - gyroBiasY;
float gz = (GyZ / GYRO_SCALE) - gyroBiasZ;
```

Durante a calibração, o sensor deve permanecer parado.

---

# Cálculo de Pitch e Roll

Os dados do acelerômetro podem ser utilizados para calcular a inclinação do sensor em relação à gravidade.

O cálculo do Pitch é realizado utilizando:

```cpp
float pitchAcc =
  atan2(
    -ax,
    sqrt(ay * ay + az * az)
  ) * 180.0 / PI;
```

O Roll é calculado utilizando:

```cpp
float rollAcc =
  atan2(
    ay,
    az
  ) * 180.0 / PI;
```

Os valores são convertidos para graus.

---

# Filtro Complementar

O acelerômetro e o giroscópio possuem características diferentes.

O acelerômetro fornece uma referência baseada na gravidade, porém pode sofrer interferência durante movimentos e vibrações.

O giroscópio responde rapidamente aos movimentos, porém pequenos erros podem se acumular ao longo do tempo.

Para combinar os dois sensores, o módulo utiliza um filtro complementar.

```cpp
const float ALPHA = 0.96;
```

O cálculo é realizado utilizando:

```cpp
pitch =
  ALPHA * (pitch + gx * dt)
  +
  (1 - ALPHA) * pitchAcc;
```

```cpp
roll =
  ALPHA * (roll + gy * dt)
  +
  (1 - ALPHA) * rollAcc;
```

O valor `ALPHA` define o equilíbrio entre os dois sensores.

Um valor maior dá mais importância ao giroscópio.

Um valor menor dá mais importância ao acelerômetro.

---

# Cálculo do Yaw

O Yaw representa a rotação no eixo Z.

O MPU6050 não possui magnetômetro, portanto não existe uma referência absoluta para esse eixo.

O valor é calculado através da integração da velocidade angular do giroscópio.

```cpp
yaw += gz * dt;
```

Com o tempo, pequenos erros podem se acumular, causando uma deriva no valor do Yaw.

Essa é uma limitação esperada ao utilizar apenas o acelerômetro e o giroscópio do MPU6050.

---

# Controle de Tempo

O módulo calcula o tempo entre cada leitura utilizando:

```cpp
unsigned long agora = millis();

float dt = (agora - tempoAnterior) / 1000.0;

tempoAnterior = agora;
```

O valor `dt` representa o intervalo de tempo, em segundos, entre uma leitura e outra.

Esse valor é utilizado principalmente para integrar os dados do giroscópio.

---

# Frequência de Atualização

O código utiliza um controle de tempo para manter aproximadamente 60 atualizações por segundo.

```cpp
const unsigned long INTERVALO_LOOP_US = 16667UL;
```

O valor corresponde aproximadamente a:

```text
1 segundo / 60 = 16,67 ms
```

O controle é realizado utilizando microssegundos.

```cpp
unsigned long tempoGastoUs =
  micros() - tempoLoopAnterior;

if (tempoGastoUs < INTERVALO_LOOP_US) {

  delayMicroseconds(
    INTERVALO_LOOP_US - tempoGastoUs
  );

}
```

---

# Dados Gerados

O módulo realiza a leitura e processamento das seguintes informações:

| Variável | Descrição | Unidade |
|---|---|---|
| `ax` | Aceleração no eixo X | g |
| `ay` | Aceleração no eixo Y | g |
| `az` | Aceleração no eixo Z | g |
| `gx` | Velocidade angular no eixo X | °/s |
| `gy` | Velocidade angular no eixo Y | °/s |
| `gz` | Velocidade angular no eixo Z | °/s |
| `pitch` | Inclinação frontal/traseira | graus |
| `roll` | Inclinação lateral | graus |
| `yaw` | Rotação no eixo Z | graus |

---

# Saída Serial

Os dados são enviados através da comunicação Serial no formato CSV.

O cabeçalho enviado é:

```text
tempo_ms,ax,ay,az,gx,gy,gz,pitch,roll,yaw
```

Um exemplo de saída seria:

```text
1520,0.0123,-0.0214,0.9987,1.2045,-0.3521,0.1023,2.4512,-1.2203,15.4832
```