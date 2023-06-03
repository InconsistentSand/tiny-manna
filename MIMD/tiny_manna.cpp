/*
Jueguito:

1) Sea h[i] el numero de granitos en el sitio i, 0<i<N-1.

2) Si h[i]>1 el sitio i esta "activo".

3) Al tiempo t, un sitio "activo" se "descarga" completamente tirando cada uno de sus granitos aleatoriamente y con igual probabilidad a la izquierda o a la derecha (el numero total de granitos entonces se conserva).

4) Los sitios se descargan sincronicamente. Entonces, a tiempo (t+1), el sitio activo i tendra h[i]=0 solo si sus vecinos no le tiraron granitos a tiempo t.

5) Se define la actividad A como el numero de sitios activos, es decir el numero de sitios que quieren descargarse.
Notar que si la densidad de granitos, [Suma_i h[i]/N] es muy baja, la actividad caera rapidamente a cero. Si la densidad es alta por otro lado, la actividad nunca cesara, ya que siempre habra sitios activos. En el medio hay una densidad "critica", para la cual la actividad decaera como una ley de potencia (pero se necesitaran sistemas grandes, y tiempos largos para verla bien definida).

*/

#include "params.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <climits>
#include <stdlib.h>
#include <immintrin.h>
#include <cstring>
#include <random>
using namespace std::chrono;
using namespace std;
#define printear(leftold) cout << _mm256_extract_epi16(leftold,0)<<" "<<_mm256_extract_epi16(leftold,1)<<" "<<_mm256_extract_epi16(leftold,2)<<" "<<_mm256_extract_epi16(leftold,3) << " " << _mm256_extract_epi16(leftold,4)<<" "<<_mm256_extract_epi16(leftold,5)<<" "<<_mm256_extract_epi16(leftold,6)<<" "<<_mm256_extract_epi16(leftold,7) << endl
#define DEBUG

typedef unsigned short * Manna_Array; // fixed-sized array


// CONDICION INICIAL ---------------------------------------------------------------
/*
Para generar una condicion inicial suficientemente uniforme con una densidad
lo mas aproximada (exacta cuando N->infinito) al numero real DENSITY, podemos hacer asi:
*/
static void inicializacion(Manna_Array h)
{
    for (short i = 0; i < N; ++i) {
        h[i] = static_cast<unsigned short>((i + 1) * DENSITY) - static_cast<unsigned short>(i * DENSITY);
    }
}


#ifdef DEBUG
static void imprimir_array(const Manna_Array h)
{
    int nrogranitos = 0;
    int nrogranitos_activos = 0;

    // esto dibuja los granitos en cada sitio y los cuenta
    for (int i = 0; i < N; ++i) {
        std::cout << h[i] << " ";
        nrogranitos += h[i];
        nrogranitos_activos += (h[i] > 1);
    }
    std::cout << "\n";
    std::cout << "Hay " << nrogranitos << " granitos en total\n";
    std::cout << "De ellos " << nrogranitos_activos << " son activos\n";
    std::cout << "La densidad obtenida es " << nrogranitos * 1.0 / N;
    std::cout << ", mientras que la deseada era " << DENSITY << "\n\n";
}

static void imprimir_dh(const Manna_Array dh)
{
    int nrogranitos = 0;

    for (int i = 0; i < N; ++i) {
        std::cout << dh[i] << " ";
        nrogranitos += dh[i];
    }
    std::cout << "\n";
    std::cout << "Hay " << nrogranitos << " granitos en total por sumar a h\n";
}



#endif


// CONDICION INICIAL ---------------------------------------------------------------
/*
El problema con la condicion inicial de arriba es que es estable, no tiene sitios activos
y por tanto no evolucionara. Hay que desestabilizarla de alguna forma.
Una forma es agarrar cada granito, y tirarlo a su izquierda o derecha aleatoriamente...
*/
static void desestabilizacion_inicial(Manna_Array h)
{
    std::vector<short> index_a_incrementar;
    for (unsigned short i = 0; i < N; ++i) {
        if (h[i] == 1) {
            h[i] = 0;
            short j = i + 2 * (rand() % 2) - 1; // izquierda o derecha

            // corrijo por condiciones periodicas
            if (j == N) {
                j = 0;
            } else if (j == -1) {
                j = N - 1;
            }

            index_a_incrementar.push_back(j);
        }
    }
    for (unsigned short i = 0; i < index_a_incrementar.size(); ++i) {
        h[index_a_incrementar[i]] += 1;
    }
}


// DESCARGA DE ACTIVOS Y UPDATE --------------------------------------------------------

#define DHSZ 16
#define NSIMD 16

static default_random_engine generator;


void randinit() {
    random_device rd;
    generator = default_random_engine(SEED ? SEED : rd());
    //~ srand(SEED ? SEED : time(NULL));
}

static inline unsigned char randchar() {
    uniform_int_distribution<unsigned char> distribution(0,255);
    return distribution(generator);
}

const __m256i maskfff0 = _mm256_set_epi32(-1,-1,-1,-1,-1,-1,-1,0);
const __m256i mask000f = _mm256_set_epi32(0,0,0,0,0,0,0,-1);

static inline __m256i shift192left(__m256i input) {
    __m256i index = _mm256_set_epi32(6,5,4,3,2,1,0,7);
    return _mm256_and_si256(mask000f, _mm256_permutevar8x32_epi32(input, index));
}

static inline __m256i shift64right(__m256i input) {
    __m256i index = _mm256_set_epi32(6,5,4,3,2,1,0,7);
    return _mm256_and_si256(maskfff0, _mm256_permutevar8x32_epi32(input, index));
}    

const __m256i zeroes = _mm256_setzero_si256();
const __m256i ones = _mm256_set1_epi16(1);

const __m256i MASK[256] = {
_mm256_blend_epi16(ones,zeroes, 0 ), _mm256_blend_epi16(ones,zeroes, 1 ), _mm256_blend_epi16(ones,zeroes, 2 ), _mm256_blend_epi16(ones,zeroes, 3 ), _mm256_blend_epi16(ones,zeroes, 4 ), _mm256_blend_epi16(ones,zeroes, 5 ), _mm256_blend_epi16(ones,zeroes, 6 ), _mm256_blend_epi16(ones,zeroes, 7 ), _mm256_blend_epi16(ones,zeroes, 8 ), _mm256_blend_epi16(ones,zeroes, 9 ), _mm256_blend_epi16(ones,zeroes, 10 ), _mm256_blend_epi16(ones,zeroes, 11 ), _mm256_blend_epi16(ones,zeroes, 12 ), _mm256_blend_epi16(ones,zeroes, 13 ), _mm256_blend_epi16(ones,zeroes, 14 ), _mm256_blend_epi16(ones,zeroes, 15 ), _mm256_blend_epi16(ones,zeroes, 16 ), _mm256_blend_epi16(ones,zeroes, 17 ), _mm256_blend_epi16(ones,zeroes, 18 ), _mm256_blend_epi16(ones,zeroes, 19 ), _mm256_blend_epi16(ones,zeroes, 20 ), _mm256_blend_epi16(ones,zeroes, 21 ), _mm256_blend_epi16(ones,zeroes, 22 ), _mm256_blend_epi16(ones,zeroes, 23 ), _mm256_blend_epi16(ones,zeroes, 24 ), _mm256_blend_epi16(ones,zeroes, 25 ), _mm256_blend_epi16(ones,zeroes, 26 ), _mm256_blend_epi16(ones,zeroes, 27 ), _mm256_blend_epi16(ones,zeroes, 28 ), _mm256_blend_epi16(ones,zeroes, 29 ), _mm256_blend_epi16(ones,zeroes, 30 ), _mm256_blend_epi16(ones,zeroes, 31 ), _mm256_blend_epi16(ones,zeroes, 32 ), _mm256_blend_epi16(ones,zeroes, 33 ), _mm256_blend_epi16(ones,zeroes, 34 ), _mm256_blend_epi16(ones,zeroes, 35 ), _mm256_blend_epi16(ones,zeroes, 36 ), _mm256_blend_epi16(ones,zeroes, 37 ), _mm256_blend_epi16(ones,zeroes, 38 ), _mm256_blend_epi16(ones,zeroes, 39 ), _mm256_blend_epi16(ones,zeroes, 40 ), _mm256_blend_epi16(ones,zeroes, 41 ), _mm256_blend_epi16(ones,zeroes, 42 ), _mm256_blend_epi16(ones,zeroes, 43 ), _mm256_blend_epi16(ones,zeroes, 44 ), _mm256_blend_epi16(ones,zeroes, 45 ), _mm256_blend_epi16(ones,zeroes, 46 ), _mm256_blend_epi16(ones,zeroes, 47 ), _mm256_blend_epi16(ones,zeroes, 48 ), _mm256_blend_epi16(ones,zeroes, 49 ), _mm256_blend_epi16(ones,zeroes, 50 ), _mm256_blend_epi16(ones,zeroes, 51 ), _mm256_blend_epi16(ones,zeroes, 52 ), _mm256_blend_epi16(ones,zeroes, 53 ), _mm256_blend_epi16(ones,zeroes, 54 ), _mm256_blend_epi16(ones,zeroes, 55 ), _mm256_blend_epi16(ones,zeroes, 56 ), _mm256_blend_epi16(ones,zeroes, 57 ), _mm256_blend_epi16(ones,zeroes, 58 ), _mm256_blend_epi16(ones,zeroes, 59 ), _mm256_blend_epi16(ones,zeroes, 60 ), _mm256_blend_epi16(ones,zeroes, 61 ), _mm256_blend_epi16(ones,zeroes, 62 ), _mm256_blend_epi16(ones,zeroes, 63 ), _mm256_blend_epi16(ones,zeroes, 64 ), _mm256_blend_epi16(ones,zeroes, 65 ), _mm256_blend_epi16(ones,zeroes, 66 ), _mm256_blend_epi16(ones,zeroes, 67 ), _mm256_blend_epi16(ones,zeroes, 68 ), _mm256_blend_epi16(ones,zeroes, 69 ), _mm256_blend_epi16(ones,zeroes, 70 ), _mm256_blend_epi16(ones,zeroes, 71 ), _mm256_blend_epi16(ones,zeroes, 72 ), _mm256_blend_epi16(ones,zeroes, 73 ), _mm256_blend_epi16(ones,zeroes, 74 ), _mm256_blend_epi16(ones,zeroes, 75 ), _mm256_blend_epi16(ones,zeroes, 76 ), _mm256_blend_epi16(ones,zeroes, 77 ), _mm256_blend_epi16(ones,zeroes, 78 ), _mm256_blend_epi16(ones,zeroes, 79 ), _mm256_blend_epi16(ones,zeroes, 80 ), _mm256_blend_epi16(ones,zeroes, 81 ), _mm256_blend_epi16(ones,zeroes, 82 ), _mm256_blend_epi16(ones,zeroes, 83 ), _mm256_blend_epi16(ones,zeroes, 84 ), _mm256_blend_epi16(ones,zeroes, 85 ), _mm256_blend_epi16(ones,zeroes, 86 ), _mm256_blend_epi16(ones,zeroes, 87 ), _mm256_blend_epi16(ones,zeroes, 88 ), _mm256_blend_epi16(ones,zeroes, 89 ), _mm256_blend_epi16(ones,zeroes, 90 ), _mm256_blend_epi16(ones,zeroes, 91 ), _mm256_blend_epi16(ones,zeroes, 92 ), _mm256_blend_epi16(ones,zeroes, 93 ), _mm256_blend_epi16(ones,zeroes, 94 ), _mm256_blend_epi16(ones,zeroes, 95 ), _mm256_blend_epi16(ones,zeroes, 96 ), _mm256_blend_epi16(ones,zeroes, 97 ), _mm256_blend_epi16(ones,zeroes, 98 ), _mm256_blend_epi16(ones,zeroes, 99 ), _mm256_blend_epi16(ones,zeroes, 100 ), _mm256_blend_epi16(ones,zeroes, 101 ), _mm256_blend_epi16(ones,zeroes, 102 ), _mm256_blend_epi16(ones,zeroes, 103 ), _mm256_blend_epi16(ones,zeroes, 104 ), _mm256_blend_epi16(ones,zeroes, 105 ), _mm256_blend_epi16(ones,zeroes, 106 ), _mm256_blend_epi16(ones,zeroes, 107 ), _mm256_blend_epi16(ones,zeroes, 108 ), _mm256_blend_epi16(ones,zeroes, 109 ), _mm256_blend_epi16(ones,zeroes, 110 ), _mm256_blend_epi16(ones,zeroes, 111 ), _mm256_blend_epi16(ones,zeroes, 112 ), _mm256_blend_epi16(ones,zeroes, 113 ), _mm256_blend_epi16(ones,zeroes, 114 ), _mm256_blend_epi16(ones,zeroes, 115 ), _mm256_blend_epi16(ones,zeroes, 116 ), _mm256_blend_epi16(ones,zeroes, 117 ), _mm256_blend_epi16(ones,zeroes, 118 ), _mm256_blend_epi16(ones,zeroes, 119 ), _mm256_blend_epi16(ones,zeroes, 120 ), _mm256_blend_epi16(ones,zeroes, 121 ), _mm256_blend_epi16(ones,zeroes, 122 ), _mm256_blend_epi16(ones,zeroes, 123 ), _mm256_blend_epi16(ones,zeroes, 124 ), _mm256_blend_epi16(ones,zeroes, 125 ), _mm256_blend_epi16(ones,zeroes, 126 ), _mm256_blend_epi16(ones,zeroes, 127 ), _mm256_blend_epi16(ones,zeroes, 128 ), _mm256_blend_epi16(ones,zeroes, 129 ), _mm256_blend_epi16(ones,zeroes, 130 ), _mm256_blend_epi16(ones,zeroes, 131 ), _mm256_blend_epi16(ones,zeroes, 132 ), _mm256_blend_epi16(ones,zeroes, 133 ), _mm256_blend_epi16(ones,zeroes, 134 ), _mm256_blend_epi16(ones,zeroes, 135 ), _mm256_blend_epi16(ones,zeroes, 136 ), _mm256_blend_epi16(ones,zeroes, 137 ), _mm256_blend_epi16(ones,zeroes, 138 ), _mm256_blend_epi16(ones,zeroes, 139 ), _mm256_blend_epi16(ones,zeroes, 140 ), _mm256_blend_epi16(ones,zeroes, 141 ), _mm256_blend_epi16(ones,zeroes, 142 ), _mm256_blend_epi16(ones,zeroes, 143 ), _mm256_blend_epi16(ones,zeroes, 144 ), _mm256_blend_epi16(ones,zeroes, 145 ), _mm256_blend_epi16(ones,zeroes, 146 ), _mm256_blend_epi16(ones,zeroes, 147 ), _mm256_blend_epi16(ones,zeroes, 148 ), _mm256_blend_epi16(ones,zeroes, 149 ), _mm256_blend_epi16(ones,zeroes, 150 ), _mm256_blend_epi16(ones,zeroes, 151 ), _mm256_blend_epi16(ones,zeroes, 152 ), _mm256_blend_epi16(ones,zeroes, 153 ), _mm256_blend_epi16(ones,zeroes, 154 ), _mm256_blend_epi16(ones,zeroes, 155 ), _mm256_blend_epi16(ones,zeroes, 156 ), _mm256_blend_epi16(ones,zeroes, 157 ), _mm256_blend_epi16(ones,zeroes, 158 ), _mm256_blend_epi16(ones,zeroes, 159 ), _mm256_blend_epi16(ones,zeroes, 160 ), _mm256_blend_epi16(ones,zeroes, 161 ), _mm256_blend_epi16(ones,zeroes, 162 ), _mm256_blend_epi16(ones,zeroes, 163 ), _mm256_blend_epi16(ones,zeroes, 164 ), _mm256_blend_epi16(ones,zeroes, 165 ), _mm256_blend_epi16(ones,zeroes, 166 ), _mm256_blend_epi16(ones,zeroes, 167 ), _mm256_blend_epi16(ones,zeroes, 168 ), _mm256_blend_epi16(ones,zeroes, 169 ), _mm256_blend_epi16(ones,zeroes, 170 ), _mm256_blend_epi16(ones,zeroes, 171 ), _mm256_blend_epi16(ones,zeroes, 172 ), _mm256_blend_epi16(ones,zeroes, 173 ), _mm256_blend_epi16(ones,zeroes, 174 ), _mm256_blend_epi16(ones,zeroes, 175 ), _mm256_blend_epi16(ones,zeroes, 176 ), _mm256_blend_epi16(ones,zeroes, 177 ), _mm256_blend_epi16(ones,zeroes, 178 ), _mm256_blend_epi16(ones,zeroes, 179 ), _mm256_blend_epi16(ones,zeroes, 180 ), _mm256_blend_epi16(ones,zeroes, 181 ), _mm256_blend_epi16(ones,zeroes, 182 ), _mm256_blend_epi16(ones,zeroes, 183 ), _mm256_blend_epi16(ones,zeroes, 184 ), _mm256_blend_epi16(ones,zeroes, 185 ), _mm256_blend_epi16(ones,zeroes, 186 ), _mm256_blend_epi16(ones,zeroes, 187 ), _mm256_blend_epi16(ones,zeroes, 188 ), _mm256_blend_epi16(ones,zeroes, 189 ), _mm256_blend_epi16(ones,zeroes, 190 ), _mm256_blend_epi16(ones,zeroes, 191 ), _mm256_blend_epi16(ones,zeroes, 192 ), _mm256_blend_epi16(ones,zeroes, 193 ), _mm256_blend_epi16(ones,zeroes, 194 ), _mm256_blend_epi16(ones,zeroes, 195 ), _mm256_blend_epi16(ones,zeroes, 196 ), _mm256_blend_epi16(ones,zeroes, 197 ), _mm256_blend_epi16(ones,zeroes, 198 ), _mm256_blend_epi16(ones,zeroes, 199 ), _mm256_blend_epi16(ones,zeroes, 200 ), _mm256_blend_epi16(ones,zeroes, 201 ), _mm256_blend_epi16(ones,zeroes, 202 ), _mm256_blend_epi16(ones,zeroes, 203 ), _mm256_blend_epi16(ones,zeroes, 204 ), _mm256_blend_epi16(ones,zeroes, 205 ), _mm256_blend_epi16(ones,zeroes, 206 ), _mm256_blend_epi16(ones,zeroes, 207 ), _mm256_blend_epi16(ones,zeroes, 208 ), _mm256_blend_epi16(ones,zeroes, 209 ), _mm256_blend_epi16(ones,zeroes, 210 ), _mm256_blend_epi16(ones,zeroes, 211 ), _mm256_blend_epi16(ones,zeroes, 212 ), _mm256_blend_epi16(ones,zeroes, 213 ), _mm256_blend_epi16(ones,zeroes, 214 ), _mm256_blend_epi16(ones,zeroes, 215 ), _mm256_blend_epi16(ones,zeroes, 216 ), _mm256_blend_epi16(ones,zeroes, 217 ), _mm256_blend_epi16(ones,zeroes, 218 ), _mm256_blend_epi16(ones,zeroes, 219 ), _mm256_blend_epi16(ones,zeroes, 220 ), _mm256_blend_epi16(ones,zeroes, 221 ), _mm256_blend_epi16(ones,zeroes, 222 ), _mm256_blend_epi16(ones,zeroes, 223 ), _mm256_blend_epi16(ones,zeroes, 224 ), _mm256_blend_epi16(ones,zeroes, 225 ), _mm256_blend_epi16(ones,zeroes, 226 ), _mm256_blend_epi16(ones,zeroes, 227 ), _mm256_blend_epi16(ones,zeroes, 228 ), _mm256_blend_epi16(ones,zeroes, 229 ), _mm256_blend_epi16(ones,zeroes, 230 ), _mm256_blend_epi16(ones,zeroes, 231 ), _mm256_blend_epi16(ones,zeroes, 232 ), _mm256_blend_epi16(ones,zeroes, 233 ), _mm256_blend_epi16(ones,zeroes, 234 ), _mm256_blend_epi16(ones,zeroes, 235 ), _mm256_blend_epi16(ones,zeroes, 236 ), _mm256_blend_epi16(ones,zeroes, 237 ), _mm256_blend_epi16(ones,zeroes, 238 ), _mm256_blend_epi16(ones,zeroes, 239 ), _mm256_blend_epi16(ones,zeroes, 240 ), _mm256_blend_epi16(ones,zeroes, 241 ), _mm256_blend_epi16(ones,zeroes, 242 ), _mm256_blend_epi16(ones,zeroes, 243 ), _mm256_blend_epi16(ones,zeroes, 244 ), _mm256_blend_epi16(ones,zeroes, 245 ), _mm256_blend_epi16(ones,zeroes, 246 ), _mm256_blend_epi16(ones,zeroes, 247 ), _mm256_blend_epi16(ones,zeroes, 248 ), _mm256_blend_epi16(ones,zeroes, 249 ), _mm256_blend_epi16(ones,zeroes, 250 ), _mm256_blend_epi16(ones,zeroes, 251 ), _mm256_blend_epi16(ones,zeroes, 252 ), _mm256_blend_epi16(ones,zeroes, 253 ), _mm256_blend_epi16(ones,zeroes, 254 ), _mm256_blend_epi16(ones,zeroes, 255 )
};


static unsigned int descargar(Manna_Array __restrict__ a, Manna_Array __restrict__ b)
{
    unsigned short * h = (unsigned short *) __builtin_assume_aligned(a, 16);
    unsigned short * dh = (unsigned short *) __builtin_assume_aligned(b, 16);
    memset(dh, 0, N*(sizeof(short)));

    // #ifdef DEBUG
    // cout << "Imprimo DH la primera vez" << endl;
    // imprimir_array(dh);
    // #endif

    unsigned short i = 0;
    unsigned short nroactivos = 0;

    short int_left = 0;
    short int_right = 0;

    // Podemos hacer el primero solo una vez, así que pido single thread
    //#pragma omp single nowait
    for (; i < NSIMD; i++) {
        short mask = (h[i] > 1) ? -1 : 0;
        int_left = mask & ((h[i] != 0) ? (rand() % h[i]) : 0);
        int_right = mask & (h[i] - int_left);
        dh[(i - 1 + N) % N] += int_left;
        dh[(i + 1) % N] += int_right;
        h[i] = (h[i] > 0) ? mask & 0 : h[i];

        short mask_prev = (i > 1) ? -1 : 0;
        h[i - 1] += mask_prev & dh[i - 1];
        nroactivos += (mask_prev & (h[i - 1] > 1));
    }

    // #ifdef DEBUG
    // cout << "Array post primer iteracion" << endl;
    // imprimir_array(h);
    // imprimir_array(dh);
    // #endif


    __m256i left = _mm256_loadu_si256((__m256i *) &dh[i-1]);
    __m256i right = zeroes;

    //#pragma omp parallel private(i, int_left, int_right) shared(nroactivos,h) num_threads(1) 
    //{
        for (; i < N - (N%NSIMD); i+=NSIMD) {
            __m256i slots = _mm256_load_si256((__m256i *) &h[i]);
            __m256i slots_gt1 = _mm256_cmpgt_epi16(slots, ones);
            
            __m256i active_slots;
            bool activity = false;

            #ifdef DEBUG
            cout << "i = " << i << endl;
            cout << "slots :" << endl;
            printear(slots);
            #endif

            while(active_slots = _mm256_and_si256(slots_gt1, _mm256_cmpgt_epi16(slots,zeroes)), _mm256_movemask_epi8(active_slots)) {
                activity = true;
                unsigned char random = randchar();
                __m256i randomright = MASK[random];
                __m256i randomleft = _mm256_xor_si256(randomright, ones);

                __m256i addright = _mm256_and_si256(randomright, active_slots);
                __m256i addleft = _mm256_and_si256(randomleft, active_slots);

                left = _mm256_adds_epu16(left, addleft);
                right = _mm256_adds_epu16(right, addright);

                slots = _mm256_subs_epu16(slots, _mm256_and_si256(active_slots, ones));       
            
                #ifdef DEBUG
                cout << "i = " << i << endl;
                cout << "slots:" << endl;
                printear(slots);
                #endif
            
            }

            #ifdef DEBUG
            cout << "slots post:" << endl;
            printear(slots);
            #endif

            __m256i shift_right = shift64right(right);
            __m256i left_to_store = _mm256_adds_epu16(left, shift_right);

            #ifdef DEBUG
            cout << "left_to_store:" << endl;
            printear(left_to_store);
            #endif

            left = shift192left(right);
            right = zeroes;

            if(activity) _mm256_store_si256((__m256i *) &h[i], slots);
            
            if(! _mm256_testz_si256(left_to_store, left_to_store)) {
            
                slots = _mm256_loadu_si256((__m256i *) &h[i-1]);
                slots = _mm256_adds_epu16(slots, left_to_store);

                _mm256_storeu_si256((__m256i *) &h[i-1], slots);

                __m256i tmp = _mm256_cmpgt_epi16(slots, ones);
                nroactivos += __builtin_popcount(_mm256_movemask_epi8(tmp))/2;
            }
            #ifdef DEBUG
            cout << "ahora h:" << endl;
            imprimir_dh(h);
            cout << "ahora dh:" << endl;
            imprimir_dh(dh);
            #endif
        }
    
    _mm256_storeu_si256((__m256i *) &dh[(i-1)%DHSZ], left);
    
    //}

    for (; i < N; i++) {
        short mask = (h[i] > 1) ? -1 : 0;
        int_left = mask & ((h[i] != 0) ? (rand() % h[i]) : 0);
        int_right = mask & (h[i] - int_left);
        dh[(i - 1 + N) % N] += int_left;
        dh[(i + 1) % N] += int_right;
        h[i] = mask & 0;

        short mask_prev = (i > 1) ? -1 : 0;
        h[i - 1] += mask_prev & dh[(i-1)%DHSZ];
        nroactivos += (mask_prev & (h[i - 1] > 1)) ? 1 : 0;
    }

    h[N - 1] = dh[(N-1)%DHSZ];
    h[0] = dh[0];
    nroactivos += (h[N - 1] > 1);
    nroactivos += (h[0] > 1);

    return nroactivos;
}


//===================================================================
int main()
{

    srand(SEED);
    randinit();

    // nro granitos en cada sitio, y su update
    Manna_Array h = (Manna_Array) aligned_alloc(128, sizeof(short)*N);
    Manna_Array dh = (Manna_Array) aligned_alloc(128, sizeof(short)*N);

    std::cout << "estado inicial estable de la pila de arena...";
    inicializacion(h);
    std::cout << "LISTO" << std::endl;

#ifdef DEBUG
    imprimir_array(h);
#endif

    std::cout << "estado inicial desestabilizado de la pila de arena..." << "\n";
    desestabilizacion_inicial(h);
    std::cout << "LISTO" << std::endl;
#ifdef DEBUG
    imprimir_array(h);
#endif

    std::cout << "evolucion de la pila de arena..." << "\n";
    std::cout.flush();

    // std::ofstream activity_out("activity.dat");
    short activity;
    int t = 0;
#ifdef METRIC
    long int max_duration = 0;
    long int sum_duration = 0;
    long int min_duration = LONG_MAX;
#endif

    do {
#ifdef METRIC
        auto start = high_resolution_clock::now();
#endif
        activity = descargar(h, dh);
#ifdef METRIC
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(stop - start).count();
        // std::cout << "Metric in iteration " << t << "(in nanoseconds): " << duration.count() << std::endl;
        sum_duration += (long int)duration;
        if ((long int)duration < min_duration) {
            min_duration = (long int)duration;
        } else if ((long int)duration > max_duration) {
            max_duration = (long int)duration;
        }
#endif
        // activity_out << activity << "\n";
#ifdef DEBUG
        imprimir_array(h);
        cout << "ahora dh" << endl;
        imprimir_dh(dh);
#endif
        ++t;
    } while (activity > 0 && t < NSTEPS); // si la actividad decae a cero, esto no evoluciona mas...

    std::cout << "LISTO: " << ((activity > 0) ? ("se acabo el tiempo\n") : ("la actividad decayo a cero\n")) << std::endl;

#ifdef METRIC
    std::cout << "METRICA\n Duracion Promedio: " << sum_duration/t << "\n Duracion Maxima: " << max_duration << "\n Duracion Minima: " << min_duration << "\n" << std::endl;
#endif

    free(h);
    free(dh);

    return 0;
}
