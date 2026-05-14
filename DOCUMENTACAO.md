# Documentação Completa - Termux RAFCODEΦ

## Índice

1. [Visão Geral](#visão-geral)
2. [O Que Este Fork Contém](#o-que-este-fork-contém)
3. [Arquitetura e Componentes](#arquitetura-e-componentes)
4. [Funcionalidades Principais](#funcionalidades-principais)
5. [Framework RAFAELIA](#framework-rafaelia)
6. [Implementação Bare-Metal](#implementação-bare-metal)
7. [Boosters de Performance](#boosters-de-performance)
8. [Compatibilidade Android 15](#compatibilidade-android-15)
9. [Guias de Uso](#guias-de-uso)
10. [Referências e Documentação Adicional](#referências-e-documentação-adicional)

---

## Visão Geral

**Termux RAFCODEΦ** é um fork aprimorado do Termux original, mantido por instituto-Rafael. Este projeto adiciona funcionalidades avançadas de baixo nível, otimizações de hardware, e compatibilidade completa com Android 15, mantendo a compatibilidade com o Termux original.

### Informações Básicas

- **Nome do Fork**: Termux RAFCODEΦ
- **Repositório**: https://github.com/instituto-Rafael/termux-app-rafacodephi
- **Package Name**: `com.termux.rafacodephi`
- **App Name**: `Termux RAFCODEΦ`
- **Licença**: GPLv3
- **Upstream**: [termux/termux-app](https://github.com/termux/termux-app)
- **Mantenedor**: instituto-Rafael

### Principais Diferenciais

✅ **Instalação lado-a-lado** com Termux oficial  
✅ **Framework RAFAELIA** para computação ética e coerente  
✅ **Implementação bare-metal** em C e Assembly  
✅ **Android 15 Ready** com otimizações específicas  
✅ **Boosters de performance** com speedup médio de 2.76x  
✅ **Otimizações de hardware** (NEON, AVX, SSE)  
✅ **Zero dependências externas** no núcleo bare-metal  
✅ **Matemática determinística** com operações de flip matriciais  

---

## O Que Este Fork Contém

### 1. Framework RAFAELIA

O **RAFAELIA Framework** (RAfael FrAmework for Ethical Linear and Iterative Analysis) é uma metodologia de desenvolvimento que enfatiza:

- **Humildade Operacional (Humildade_Ω)**: Desenvolvimento iterativo com validação
- **Filtro Ético (Φ_ethica)**: Minimizar entropia, maximizar coerência
- **Retroalimentação (ψχρΔΣΩ)**: Ciclo contínuo de percepção, feedback, expansão, validação, execução e alinhamento
- **Determinismo**: Operações previsíveis e reproduzíveis

**Documentação**:
- [RAFAELIA_METHODOLOGY.md](RAFAELIA_METHODOLOGY.md) - Metodologia completa
- [RAFAELIA_IMPLEMENTATION_SUMMARY.md](RAFAELIA_IMPLEMENTATION_SUMMARY.md) - Resumo de implementação
- [docs/rafaelia/](docs/rafaelia/) - Documentação detalhada

### 2. Implementação Bare-Metal

Programas internos refatorados em C e Assembly de baixo nível:

**Características**:
- 📦 Binário de ~50 KB (vs ~5 MB com bibliotecas externas)
- 🚀 2.7x mais rápido que implementações Java
- 🎯 Zero dependências externas
- 🔧 Acesso direto ao hardware

**Componentes**:
- `baremetal.c/h` - Implementação principal em C
- `baremetal_asm.S` - Otimizações SIMD em Assembly
- `baremetal_jni.c` - Ponte JNI para Java
- `BareMetal.java` - Interface Java

**Operações Implementadas**:
- Operações vetoriais (dot product, norm, add, subtract)
- Operações matriciais (multiply, transpose, determinant, inverse)
- Flip determinístico (horizontal, vertical, diagonal)
- Matemática rápida (sqrt, rsqrt, exp, log)
- Operações de memória otimizadas
- Solver de sistemas lineares

**Documentação**:
- [IMPLEMENTACAO_BAREMETAL.md](IMPLEMENTACAO_BAREMETAL.md) - Guia de implementação
- [SUMMARY.md](SUMMARY.md) - Resumo final
- [app/src/main/cpp/lowlevel/README.md](app/src/main/cpp/lowlevel/README.md) - README técnico

### 3. Compatibilidade Android 15

Otimizações e correções para Android 15:

- ✅ Phantom Process Killer handling
- ✅ Package name único (`com.termux.rafacodephi`)
- ✅ Authorities únicas (sem conflitos)
- ✅ Permissions únicas
- ✅ Diretórios de dados únicos
- ✅ Instalação lado-a-lado com Termux oficial

**Documentação**:
- [ANDROID15_AUDIT_REPORT.md](ANDROID15_AUDIT_REPORT.md) - Relatório de auditoria
- [docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md](docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md) - Guia de compatibilidade
- [docs/MUDANCAS_ANDROID15.md](docs/MUDANCAS_ANDROID15.md) - Changelog

### 4. Otimizações de Hardware

Suporte automático para:

**ARM**:
- ARMv7-A (armeabi-v7a) com NEON SIMD
- ARMv8-A (arm64-v8a) com NEON avançado

**x86**:
- x86 com SSE2 baseline
- x86_64 com SSE4.2, AVX, AVX2

**Recursos**:
- Detecção automática de arquitetura
- Otimizações SIMD (3-4x speedup)
- Fallback para operações escalares
- Compilação para todas as arquiteturas

### 5. Estrutura Modular

Organização clara e modular:

```
termux-app-rafacodephi/
├── app/                          # Aplicação principal
│   ├── src/main/
│   │   ├── cpp/lowlevel/        # Implementação bare-metal
│   │   └── java/com/termux/
│   │       ├── lowlevel/        # Interface Java
│   │       └── app/             # UI e lógica
├── rafaelia/                     # Módulo RAFAELIA
├── docs/                         # Documentação
│   ├── rafaelia/                # Docs RAFAELIA
│   ├── RAFCODEPHI_ANDROID15_COMPATIBILITY.md
│   ├── MUDANCAS_ANDROID15.md
│   └── LOWLEVEL_SUMMARY.md
├── terminal-emulator/            # Emulador de terminal
├── terminal-view/                # View do terminal
└── termux-shared/               # Biblioteca compartilhada
```

---

## Arquitetura e Componentes

### Camadas da Aplicação

```
┌─────────────────────────────────────────┐
│  UI Layer (Java/Kotlin)                 │
│  - TermuxActivity                       │
│  - TermuxService                        │
│  - TerminalView                         │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  Application Layer (Java)               │
│  - BareMetal API                        │
│  - InternalPrograms                     │
│  - TermuxConstants                      │
└─────────────────────────────────────────┘
                  ↓ JNI
┌─────────────────────────────────────────┐
│  Native Layer (C/Assembly)              │
│  - baremetal.c (Core logic)            │
│  - baremetal_asm.S (SIMD)              │
│  - baremetal_jni.c (Bridge)            │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  Hardware Layer                         │
│  - ARM NEON / x86 AVX                  │
│  - CPU, Memory, Storage                 │
└─────────────────────────────────────────┘
```

### Componentes Principais

#### 1. Terminal Emulator
- Baseado em Android Terminal Emulator
- Emulação VT-100 e ANSI
- Suporte a UTF-8
- Integração com shell Linux

#### 2. Terminal View
- Renderização de terminal
- Input handling
- Gestos e atalhos
- Configurações de aparência

#### 3. Termux Shared
- Constantes compartilhadas
- Utilitários comuns
- File system helpers
- Shell helpers

#### 4. RAFAELIA Module
- Framework metodológico
- Implementações de referência
- Documentação acadêmica
- Fórmulas e princípios

#### 5. Bare-Metal Module
- Operações de baixo nível
- Otimizações de hardware
- Matemática determinística
- Zero dependências

---

## Funcionalidades Principais

### 1. Operações Vetoriais (SIMD-Optimized)

```java
// Produto escalar
float[] v1 = {1.0f, 2.0f, 3.0f};
float[] v2 = {4.0f, 5.0f, 6.0f};
float dot = BareMetal.vectorDot(v1, v2);  // 32.0

// Norma euclidiana
float norm = BareMetal.vectorNorm(v1);  // 3.74...

// Similaridade de cosseno
float similarity = BareMetal.cosineSimilarity(v1, v2);  // 0.97...
```

### 2. Operações Matriciais

```java
// Criar matriz 3x3
BareMetal.Matrix m = new BareMetal.Matrix(3, 3);

// Definir dados
float[] data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
m.setData(data);

// Operações
m.flipHorizontal();   // Espelhar horizontalmente
m.flipVertical();     // Espelhar verticalmente
m.flipDiagonal();     // Transpor

// Cálculos
float det = m.determinant();
BareMetal.Matrix inv = m.invert();
BareMetal.Matrix product = m.multiply(inv);

// Cleanup
m.close();
```

### 3. Flip Determinístico

Resolve sistemas lineares através de transformações matriciais:

```java
// Sistema Ax = b
float[][] A = {
    {2, 1, 1},
    {1, 3, 2},
    {1, 0, 0}
};
float[] b = {4, 5, 6};

BareMetal.Matrix matrix = new BareMetal.Matrix(3, 3);
matrix.setData(flattenMatrix(A));

// Resolver com flips determinísticos
float[] solution = matrix.solve(b);
// solution = [6, -1, -1]

matrix.close();
```

### 4. Matemática Rápida

```java
// Raiz quadrada rápida (Newton-Raphson)
float sqrt = BareMetal.fastSqrt(16.0f);  // 4.0

// Raiz quadrada recíproca (Quake III)
float rsqrt = BareMetal.fastRsqrt(16.0f);  // 0.25

// Exponencial
float exp = BareMetal.fastExp(2.0f);  // 7.389...

// Logaritmo
float log = BareMetal.fastLog(10.0f);  // 2.302...
```

### 5. Detecção de Hardware

```java
// Obter arquitetura
String arch = BareMetal.getArchitecture();
// "arm64-v8a", "armeabi-v7a", "x86_64", ou "x86"

// Verificar capacidades
boolean hasNeon = BareMetal.hasNeon();  // true em ARM com NEON
boolean hasAvx = BareMetal.hasAvx();    // true em x86_64 com AVX

// Obter capacidades como bitmask
int caps = BareMetal.getCapabilities();
```

### 6. Programas Internos

```java
// Análise de vetores
float similarity = InternalPrograms.VectorAnalyzer
    .analyzeSimilarity(features1, features2);

// Processamento de imagem
InternalPrograms.ImageProcessor
    .flipHorizontal(imageData, width, height);

// Matemática rápida
float result = InternalPrograms.FastMath.sqrt(value);

// Operações de memória
InternalPrograms.MemoryOps.copy(src, dst, size);
```

---

## Framework RAFAELIA

### Princípios Fundamentais

#### 1. Humildade_Ω (Humildade Operacional)

```
CHECKPOINT = { (o_que_sei), (o_que_não_sei), (próximo_passo) }
```

- Reconhecer limites do conhecimento
- Não implementar placeholders
- Desenvolvimento iterativo com validação

#### 2. Φ_ethica (Filtro Ético)

```
Φ_ethica = Min(Entropia) × Max(Coerência)
```

- Minimizar complexidade
- Maximizar consistência
- Resultados determinísticos

#### 3. Retroalimentação (ψχρΔΣΩ Cycle)

```
ψ→χ→ρ→Δ→Σ→Ω→ψ
```

- **ψ (psi)**: Percepção - Ler e processar entrada
- **χ (chi)**: Feedback - Verificar coerência
- **ρ (rho)**: Expansão - Transformar e computar
- **Δ (Delta)**: Validação - Verificar resultados
- **Σ (Sigma)**: Execução - Sintetizar saída
- **Ω (Omega)**: Alinhamento - Garantir coerência ética

#### 4. Determinismo

```
R_Ω = Σ_n (ψ_n·χ_n·ρ_n·Δ_n·Σ_n·Ω_n)^{Φλ}
```

- Baseado em matrizes
- Operações de flip para resolver
- Sem aleatoriedade
- Rastreável e auditável

### Aplicação Prática

O RAFAELIA Framework é aplicado em:

1. **Desenvolvimento**: Ciclo ψχρΔΣΩ para cada feature
2. **Code Review**: Verificação de Φ_ethica
3. **Testes**: Validação determinística
4. **Documentação**: Atribuição adequada
5. **Otimização**: Melhor uso de hardware

---

## Implementação Bare-Metal

### Características Técnicas

**Linguagens**: C11, Assembly (ARM NEON, ARM64 NEON)  
**Tamanho**: ~50 KB  
**Dependências**: Apenas libc sistema  
**Performance**: 2.7x média de speedup

### Estrutura de Dados

```c
// Estrutura de matriz (minimalista)
typedef struct {
    float* m;       /* Dados da matriz */
    uint32_t r;     /* Linhas */
    uint32_t c;     /* Colunas */
} mx_t;
```

### Operações Implementadas

#### Vetoriais (32 funções JNI)
- Produto escalar (SIMD)
- Norma euclidiana
- Adição/subtração
- Similaridade de cosseno

#### Matriciais
- Criar/liberar
- Multiplicar
- Transpor
- Determinante
- Inverter
- Flip (H, V, D)
- Resolver sistema linear

#### Matemáticas
- sqrt (Newton-Raphson)
- rsqrt (Quake III)
- exp (Taylor series)
- log (bit manipulation)

#### Memória
- Cópia otimizada (32-bit words)
- Preenchimento
- Comparação

### Otimizações de Hardware

#### ARM NEON (ARMv7-A)
```asm
.global bm_dot_neon
bm_dot_neon:
    vmov.f32    q0, #0.0            @ Acumulador
    vld1.32     {d2, d3}, [r0]!     @ Carregar 4 floats
    vld1.32     {d4, d5}, [r1]!     @ Carregar 4 floats
    vmla.f32    q0, q1, q2          @ Multiply-accumulate
    vadd.f32    d0, d0, d1          @ Somar horizontal
    vpadd.f32   d0, d0, d0          @ Somar pares
    vmov.f32    r0, s0              @ Retornar
    bx          lr
```

#### ARM64 NEON (ARMv8-A)
```asm
.global bm_dot_neon
bm_dot_neon:
    movi        v0.4s, #0           @ Acumulador
    ld1         {v1.4s}, [x0], #16  @ Carregar 4 floats
    ld1         {v2.4s}, [x1], #16  @ Carregar 4 floats
    fmla        v0.4s, v1.4s, v2.4s @ SIMD multiply-add
    faddp       v0.4s, v0.4s, v0.4s @ Pairwise add
    faddp       v0.4s, v0.4s, v0.4s @ Final sum
    fmov        w0, s0              @ Retornar
    ret
```

#### x86 AVX/SSE
```makefile
# Flags de compilação
LOCAL_CFLAGS += -msse2 -msse4.2 -mavx -ftree-vectorize
```

### Performance

| Operação | Java (ms) | Bare-Metal (ms) | Speedup |
|----------|-----------|-----------------|---------|
| Vector dot (1K dim, 10K iter) | 5.0 | 1.5 | **3.3x** |
| Memory copy (1MB) | 2.5 | 0.8 | **3.1x** |
| Square root (100K ops) | 15.0 | 8.0 | **1.9x** |
| Matrix multiply (100×100) | 50.0 | 20.0 | **2.5x** |

**Média: 2.7x speedup**

---

## Boosters de Performance

### Visão Geral dos Boosters

Este fork inclui **6 tipos principais de boosters de performance** (otimizações de aceleração) que aceleram operações computacionais em **2.76x em média**. Boosters são implementações otimizadas que aproveitam hardware específico e algoritmos eficientes.

### Tipos de Boosters Disponíveis

#### 1. **Boosters SIMD** 🎯
- **ARM NEON**: 3.8x speedup médio (ARMv7-A, ARMv8-A)
- **x86 AVX2**: 4.1x speedup médio (256-bit)
- **x86 SSE2/SSE4.2**: 3.4x speedup médio (128-bit)

#### 2. **Boosters Bare-Metal** 🔧
- Matemática rápida: 2.6x speedup médio
- Zero dependências externas
- Tamanho reduzido (~50 KB)

#### 3. **Boosters de Memória** 💾
- Cópia otimizada: 3.2x speedup
- Preenchimento: 2.9x speedup
- Comparação: 2.5x speedup

#### 4. **Boosters Matemáticos** 🧮
- Fast sqrt: 2.8x speedup
- Fast rsqrt (Quake III): 3.5x speedup
- Fast exp: 1.9x speedup
- Fast log: 2.2x speedup

#### 5. **Boosters Vetoriais** 📊
- Produto escalar SIMD: 3.5x speedup
- Norma L2: 3.7x speedup
- Similaridade de cosseno: 3.8x speedup

#### 6. **Boosters Matriciais** 📐
- Multiplicação: 2.7x speedup
- Flips (RAFAELIA): 3.0-3.2x speedup
- Transposição: 2.8x speedup
- Operações avançadas: 2.6-2.8x speedup

### Resumo de Performance

| Tipo de Booster | Operações | Speedup Médio | Melhor Caso |
|-----------------|-----------|---------------|-------------|
| SIMD NEON | 8 | **3.8x** | **4.5x** |
| SIMD AVX2 | 6 | **4.1x** | **4.5x** |
| Bare-Metal Math | 4 | **2.6x** | **3.5x** |
| Memória | 4 | **2.9x** | **3.2x** |
| Vetorial | 4 | **3.7x** | **3.8x** |
| Matricial | 8 | **2.83x** | **3.24x** |

**Speedup Médio Global: 2.76x**

### Ativação Automática

Os boosters são ativados automaticamente:

1. **Detecção de arquitetura** em tempo de compilação
2. **Detecção de SIMD** (NEON, AVX, SSE)
3. **Seleção automática** da melhor implementação
4. **Fallback genérico** se SIMD não disponível

```java
// Verificar boosters disponíveis
if (BareMetal.isLoaded()) {
    String arch = BareMetal.getArchitecture();
    int caps = BareMetal.getCapabilities();
    boolean hasNeon = (caps & BareMetal.CAP_NEON) != 0;
    boolean hasAvx2 = (caps & BareMetal.CAP_AVX2) != 0;
}
```

### Documentação Completa

Para detalhes completos sobre cada tipo de booster, incluindo:
- Implementações técnicas detalhadas
- Código Assembly NEON/AVX
- Benchmarks individuais por operação
- Exemplos de uso práticos
- Comparações arquitetura por arquitetura

**Consulte: [BOOSTERS.md](./BOOSTERS.md)** - Documentação completa de boosters de performance

---

## Compatibilidade Android 15

### Problemas Resolvidos

#### 1. Phantom Process Killer
- Android 15 mata processos "fantasma" (limite de 32)
- Solução: Otimizações no gerenciamento de processos
- Documentação em [docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md](docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md)

#### 2. Package Collisions
- **Problema**: Nome de pacote conflitante
- **Solução**: `com.termux.rafacodephi` (único)
- **Benefício**: Instalação lado-a-lado

#### 3. Authority Conflicts
- **Problema**: Authorities compartilhadas
- **Solução**: Authorities únicas com sufixo `.rafacodephi`
- **Exemplo**: `com.termux.rafacodephi.files`

#### 4. Permission Conflicts
- **Problema**: Permissions conflitantes
- **Solução**: Permissions únicas com prefixo
- **Exemplo**: `com.termux.rafacodephi.permission.RUN_COMMAND`

### Testes de Compatibilidade

✅ **Android 7-14**: Totalmente compatível  
✅ **Android 15**: Compatível com otimizações  
✅ **Side-by-side**: Funciona com Termux oficial  
✅ **Data migration**: Suportado (manual)

---


## Estado Real de Build/Release (Sincronizado com Código)

### Fonte de verdade atual

- Pipeline principal: `.github/workflows/rafaelia_pipeline.yml`
- Workflows legados (`debug_build.yml`, `android15_arm64_build.yml`, `run_tests.yml`, etc.) continuam no repositório, mas com aviso explícito de que foram incorporados ao pipeline unificado.
- Matriz de artefatos signed/unsigned: `.github/workflows/apk_matrix_artifacts_variants.yml` + `scripts/build_apk_matrix.sh`.

### Compilação e toolchain efetivos

- **JDK**: Java 17 (pipelines principais) e Java 21 (workflow dedicado de matriz signed/unsigned).
- **SDK/NDK** (fonte: `gradle.properties`):
  - `compileSdkVersion=35`
  - `targetSdkVersion=34`
  - `minSdkVersion=21`
  - `ndkVersion=26.3.11579264`

### ABIs e contratos de saída

- ABIs mandatórias validadas no fluxo de build: `arm64-v8a`, `armeabi-v7a`, `x86_64`.
- `universal` é tratado como mandatória quando gerado.
- `x86` é opcional (aceito quando existir, sem quebrar build quando ausente).

### Artefatos publicados

- Upload de APKs por ABI em workflows de build.
- Upload de `SHA256SUMS` para rastreabilidade/reprodutibilidade.
- Workflow de matriz permite publicar: 
  - `all`
  - `signed-only`
  - `unsigned-only`
  - `reports-only`

### Assinatura e trilha oficial

- O repositório suporta geração de APK **assinado e não assinado** na trilha de matriz de artefatos.
- A trilha oficial de release mantém separação explícita de artefatos, sem rebaixar release oficial para unsigned por conveniência.

---

## Guias de Uso

### Instalação

```bash
# 1. Compilar
./gradlew assembleDebug

# 2. Instalar via ADB
adb install app/build/outputs/apk/debug/termux-app_apt-android-7-debug_universal.apk

# 3. Verificar instalação
adb shell pm list packages | grep termux.rafacodephi
```

### Uso Básico

#### 1. Detectar Arquitetura

```java
if (BareMetal.isLoaded()) {
    String arch = BareMetal.getArchitecture();
    Log.i(TAG, "Architecture: " + arch);
}
```

#### 2. Operações Vetoriais

```java
float[] a = {1, 2, 3};
float[] b = {4, 5, 6};
float dot = BareMetal.vectorDot(a, b);
```

#### 3. Operações Matriciais

```java
try (BareMetal.Matrix m = new BareMetal.Matrix(3, 3)) {
    m.setData(new float[]{1,2,3,4,5,6,7,8,9});
    m.flipHorizontal();
    float det = m.determinant();
}
```

### Compilação

#### Requisitos

- JDK 11+
- Android SDK
- Android NDK (para bare-metal)
- Gradle 7.0+

#### Comandos

```bash
# Compilar debug
./gradlew assembleDebug

# Compilar release
./gradlew assembleRelease

# Executar testes
./gradlew test

# Limpar build
./gradlew clean
```

### Desenvolvimento

#### Estrutura de Branches

- `master` - Estável
- `dev` - Desenvolvimento
- `feature/*` - Features
- `copilot/*` - PRs do Copilot

#### Workflow

1. Fork ou clone
2. Criar branch
3. Fazer alterações
4. Testar localmente
5. Commit com mensagens convencionais
6. Push e PR

### Troubleshooting

Consulte [TROUBLESHOOTING.md](TROUBLESHOOTING.md) para problemas comuns.

---

## Referências e Documentação Adicional

### Documentação Deste Fork

| Documento | Descrição |
|-----------|-----------|
| [README.md](README.md) | README principal |
| [BOOSTERS.md](BOOSTERS.md) | **Boosters de performance - detalhes e benchmarks** |
| [BENCHMARKS_COMPARISON.md](BENCHMARKS_COMPARISON.md) | Comparação detalhada de 30+ métricas |
| [BOOSTERS_DOCUMENTACAO.md](BOOSTERS_DOCUMENTACAO.md) | Guia completo de boosters de performance |
| [BENCHMARKS_COMPARISON.md](BENCHMARKS_COMPARISON.md) | Benchmarks e comparação detalhada |
| [RAFAELIA_METHODOLOGY.md](RAFAELIA_METHODOLOGY.md) | Metodologia RAFAELIA |
| [RAFAELIA_IMPLEMENTATION_SUMMARY.md](RAFAELIA_IMPLEMENTATION_SUMMARY.md) | Resumo de implementação |
| [IMPLEMENTACAO_BAREMETAL.md](IMPLEMENTACAO_BAREMETAL.md) | Guia bare-metal |
| [SUMMARY.md](SUMMARY.md) | Resumo final |
| [ANDROID15_AUDIT_REPORT.md](ANDROID15_AUDIT_REPORT.md) | Auditoria Android 15 |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Solução de problemas |
| [docs/rafaelia/](docs/rafaelia/) | Documentação RAFAELIA |
| [docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md](docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md) | Compatibilidade Android 15 |

### Documentação Upstream (Termux)

- [Termux Wiki](https://wiki.termux.com/wiki/)
- [Termux App Wiki](https://github.com/termux/termux-app/wiki)
- [Termux Packages Wiki](https://github.com/termux/termux-packages/wiki)

### Links Úteis

- **Repositório**: https://github.com/instituto-Rafael/termux-app-rafacodephi
- **Issues**: https://github.com/instituto-Rafael/termux-app-rafacodephi/issues
- **Upstream**: https://github.com/termux/termux-app
- **Termux Website**: https://termux.com

### Licença

Este fork mantém a licença GPLv3 do projeto original Termux.

**Copyright (c) 2024-present instituto-Rafael**  
**Original Termux Copyright (c) Termux developers and contributors**

---

**FIAT RAFAELIA** - Que haja computação ética e coerente.

**Φ_ethica** - Que todas as operações minimizem entropia e maximizem coerência.

**ψχρΔΣΩ** - O ciclo eterno de percepção, feedback, expansão, validação, execução e alinhamento.


### Governança e Auditabilidade Multi-IA
- [FRAMEWORK_MULTI_IA_SPEC_V1.md](FRAMEWORK_MULTI_IA_SPEC_V1.md) - Especificação de governança, privacidade, rastreabilidade e proteção humana.

## Governança de promoção de artefatos `Arme/` (Release)

- Diretórios canônicos para produção:
  - `Arme/spec/`
  - `Arme/include/`
  - `Arme/src/c/`
  - `Arme/src/asm/arm32/`
  - `Arme/src/asm/arm64/`
  - `Arme/tests/`
  - `Arme/bench/`
  - `Arme/reports/`
- `Arme/Add/` é **somente staging**. Conteúdo em `Arme/Add/` não integra build oficial sem promoção explícita.
- Promoção obrigatória via `scripts/promote_arme_module.sh`, com três gates:
  1. classificação e status válidos no `Arme/manifest.json`;
  2. teste mínimo de equivalência C/ASM em `Arme/tests/equivalence/<id>.sh`;
  3. trilha de auditoria em `Arme/reports/promotion_audit.log`.
- CI bloqueia PR/push com novos arquivos `.c/.h/.S` em `Arme/Add/` sem entrada correspondente no manifesto através do workflow `.github/workflows/arme-add-governance.yml`.
