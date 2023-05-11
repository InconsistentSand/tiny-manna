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
using namespace std::chrono;

typedef std::array<short, N> Manna_Array; // fixed-sized array


// CONDICION INICIAL ---------------------------------------------------------------
/*
Para generar una condicion inicial suficientemente uniforme con una densidad
lo mas aproximada (exacta cuando N->infinito) al numero real DENSITY, podemos hacer asi:
*/
static void inicializacion(Manna_Array& h)
{
    for (short i = 0; i < N; ++i) {
        h[i] = static_cast<short>((i + 1) * DENSITY) - static_cast<short>(i * DENSITY);
    }
}


#ifdef DEBUG
static void imprimir_array(const Manna_Array& h)
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
#endif


// CONDICION INICIAL ---------------------------------------------------------------
/*
El problema con la condicion inicial de arriba es que es estable, no tiene sitios activos
y por tanto no evolucionara. Hay que desestabilizarla de alguna forma.
Una forma es agarrar cada granito, y tirarlo a su izquierda o derecha aleatoriamente...
*/
static void desestabilizacion_inicial(Manna_Array& h)
{
    std::vector<short> index_a_incrementar;
    for (short i = 0; i < N; ++i) {
        if (h[i] == 1) {
            h[i] = 0;
            short j = i + 2 * (rand() % 2) - 1; // izquierda o derecha

            // corrijo por condiciones periodicas
            if (j == N) {
            } else if (j == -1) {
                j = 0;
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
static unsigned int descargar(Manna_Array& h, Manna_Array& dh)
{
    dh.fill(0);

    short left = 0;
    short right = 0;

    unsigned short nroactivos = 0;
    for (short i = 0; i < N; ++i) {
        // si es activo lo descargo aleatoriamente
        if (h[i] > 1) {
            left = rand() % h[i];
            right = h[i] - left;
            dh[(i-1+N)%N] += left;
            dh[(i+1)%N] += right;

            h[i] = 0;
        }
        if (i > 1) {
            h[i-1] += dh[i-1];
            nroactivos += (h[i-1] > 1);
        }
    }

    h[N-1] = dh[N-1];
    h[0] = dh[0];
    nroactivos += (h[N-1] > 1) + (h[0] > 1);

    return nroactivos;
}


//===================================================================
int main()
{

    srand(SEED);

    // nro granitos en cada sitio, y su update
    Manna_Array h, dh;

    std::cout << "estado inicial estable de la pila de arena...";
    inicializacion(h);
    std::cout << "LISTO" << std::endl;

#ifdef DEBUG
    imprimir_array(h);
#endif

    std::cout << "estado inicial desestabilizado de la pila de arena...";
    desestabilizacion_inicial(h);
    std::cout << "LISTO" << std::endl;
#ifdef DEBUG
    imprimir_array(h);
#endif

    std::cout << "evolucion de la pila de arena...";
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
#endif
        ++t;
    } while (activity > 0 && t < NSTEPS); // si la actividad decae a cero, esto no evoluciona mas...

    std::cout << "LISTO: " << ((activity > 0) ? ("se acabo el tiempo\n") : ("la actividad decayo a cero\n")) << std::endl;

#ifdef METRIC
    std::cout << "METRICA\n Duracion Promedio: " << sum_duration/t << "\n Duracion Maxima: " << max_duration << "\n Duracion Minima: " << min_duration << "\n" << std::endl;
#endif

    return 0;
}
