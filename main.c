#include <stdio.h>
#include <stdlib.h>


typedef struct Relacja {
    int kto_sie_podoba;
    int jak_bardzo;
    struct Relacja* nastepna;
} Relacja;

typedef struct Swatka {
    int liczba_klientow;
    Relacja** preferencje_klientow;
    int* plcie;

    int liczba_wierszy;
    int liczba_kolumn;
    int czy_obracamy;

    int** wspolrzedne_potencjalow;
    int** oryginalne_wspolrzedne_potencjaly;
    int* macierz_potencjalow;
    int* oryginalna_macierz_potencjalow;

    int* para_w_wierszu;
    int* para_w_kolumnie;
} Swatka;

void inicjalizuj_swatke(Swatka* swatka, int n) {
    swatka->liczba_klientow = n;
    swatka->preferencje_klientow = (Relacja**)calloc(n, sizeof(Relacja*));
    swatka->plcie = (int*)calloc(n, sizeof(int));
    
    swatka->wspolrzedne_potencjalow = NULL;
    swatka->oryginalne_wspolrzedne_potencjaly = NULL;
    swatka->macierz_potencjalow = NULL;
    swatka->oryginalna_macierz_potencjalow = NULL;
    swatka->para_w_wierszu = NULL;
    swatka->para_w_kolumnie = NULL;
}

void dodaj_relacje(Swatka* swatka, int u, int v, int jak_bardzo) {
    Relacja* nowa_u = (Relacja*)malloc(sizeof(Relacja));
    nowa_u->kto_sie_podoba = v;
    nowa_u->jak_bardzo = jak_bardzo;
    nowa_u->nastepna = swatka->preferencje_klientow[u];
    swatka->preferencje_klientow[u] = nowa_u;

    Relacja* nowa_v = (Relacja*)malloc(sizeof(Relacja));
    nowa_v->kto_sie_podoba = u;
    nowa_v->jak_bardzo = jak_bardzo;
    nowa_v->nastepna = swatka->preferencje_klientow[v];
    swatka->preferencje_klientow[v] = nowa_v;
}

void podziel_na_grupy(Swatka* swatka) {
    int n = swatka->liczba_klientow;
    int* kolejka = (int*)malloc(n * sizeof(int));
    int poczatek = 0, koniec = 0;

    for (int i = 0; i < n; i++) {
        if (swatka->plcie[i] == 0) {
            swatka->plcie[i] = 1;
            kolejka[koniec++] = i;

            while (poczatek < koniec) {
                int obecny = kolejka[poczatek++];
                Relacja* sasiad = swatka->preferencje_klientow[obecny];
                
                while (sasiad != NULL) {
                    int kto_sie_podoba = sasiad->kto_sie_podoba;
                    if (swatka->plcie[kto_sie_podoba] == 0) {
                        swatka->plcie[kto_sie_podoba] = (swatka->plcie[obecny] == 1) ? 2 : 1;
                        kolejka[koniec++] = kto_sie_podoba;
                    } 
                    sasiad = sasiad->nastepna;
                }
            }
        }
    }
    free(kolejka);
}

void zbuduj_macierz(Swatka* swatka) {
    int n = swatka->liczba_klientow;
    int* id_wiersza = (int*)calloc(n, sizeof(int));
    int* id_kolumny = (int*)calloc(n, sizeof(int));
    int wiersze = 0, kolumny = 0;

    for (int i = 0; i < n; i++) {
        if (swatka->plcie[i] == 1) id_wiersza[i] = wiersze++;
        else id_kolumny[i] = kolumny++;
    }

    swatka->liczba_wierszy = wiersze;
    swatka->liczba_kolumn = kolumny;
    swatka->czy_obracamy = 0;

    if (swatka->liczba_wierszy > swatka->liczba_kolumn) {
        swatka->liczba_wierszy = kolumny;
        swatka->liczba_kolumn = wiersze;
        swatka->czy_obracamy = 1;
    }

    int N = swatka->liczba_wierszy;
    int M = swatka->liczba_kolumn;

    swatka->para_w_wierszu = (int*)malloc(N * sizeof(int));
    swatka->para_w_kolumnie = (int*)malloc(M * sizeof(int));
    for (int i = 0; i < N; i++) swatka->para_w_wierszu[i] = -1;
    for (int j = 0; j < M; j++) swatka->para_w_kolumnie[j] = -1;

    swatka->wspolrzedne_potencjalow = (int**)malloc(N * sizeof(int*));
    swatka->oryginalne_wspolrzedne_potencjaly = (int**)malloc(N * sizeof(int*));
    swatka->macierz_potencjalow = (int*)calloc(N * M, sizeof(int));
    swatka->oryginalna_macierz_potencjalow = (int*)calloc(N * M, sizeof(int));

    for (int i = 0; i < N; i++) {
        swatka->wspolrzedne_potencjalow[i] = &swatka->macierz_potencjalow[i * M];
        swatka->oryginalne_wspolrzedne_potencjaly[i] = &swatka->oryginalna_macierz_potencjalow[i * M];
    }

    for (int i = 0; i < n; i++) {
        if (swatka->plcie[i] == 1) {
            Relacja* k = swatka->preferencje_klientow[i];
            while (k != NULL) {
                int j = k->kto_sie_podoba;
                int r = id_wiersza[i];
                int c = id_kolumny[j];

                if (swatka->czy_obracamy) {
                    swatka->wspolrzedne_potencjalow[c][r] = k->jak_bardzo;
                    swatka->oryginalne_wspolrzedne_potencjaly[c][r] = k->jak_bardzo;
                } else {
                    swatka->wspolrzedne_potencjalow[r][c] = k->jak_bardzo;
                    swatka->oryginalne_wspolrzedne_potencjaly[r][c] = k->jak_bardzo;
                }
                k = k->nastepna;
            }
        }
    }

    free(id_wiersza);
    free(id_kolumny);
}


void przygotuj_koszty_i_heurystyke(Swatka* swatka) {
    int N = swatka->liczba_wierszy;
    int M = swatka->liczba_kolumn;

    for (int i = 0; i < N; i++) {
        int max_waga = 0;
        for (int j = 0; j < M; j++) {
            if (swatka->wspolrzedne_potencjalow[i][j] > max_waga) {
                max_waga = swatka->wspolrzedne_potencjalow[i][j];
            }
        }
        for (int j = 0; j < M; j++) {
            swatka->wspolrzedne_potencjalow[i][j] = max_waga - swatka->wspolrzedne_potencjalow[i][j];
        }
    }

    if (N == M) {
        int* min_w_kolumnie = (int*)malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) min_w_kolumnie[j] = 2147483647;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (swatka->wspolrzedne_potencjalow [i][j] < min_w_kolumnie[j]) {
                    min_w_kolumnie[j] = swatka->wspolrzedne_potencjalow[i][j];
                }
            }
        }
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                swatka->wspolrzedne_potencjalow[i][j] -= min_w_kolumnie[j];
            }
        }
        free(min_w_kolumnie);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (swatka->wspolrzedne_potencjalow[i][j] == 0 && swatka->para_w_kolumnie[j] == -1 && swatka->para_w_wierszu[i] == -1) {
                swatka->para_w_kolumnie[j] = i;
                swatka->para_w_wierszu[i] = j;
                break;
            }
        }
    }
}


void szukaj_sciezki_wegierskiej(Swatka* swatka) {
    int N = swatka->liczba_wierszy;
    int M = swatka->liczba_kolumn;
    int INF = 2137694200;

    int* potencjal_wiersza = (int*)calloc(N, sizeof(int));    
    int* potencjal_kolumny = (int*)calloc(M, sizeof(int));
    int* luz = (int*)calloc(M, sizeof(int));
    int* skad_przyszlismy = (int*)calloc(M, sizeof(int));
    
    int* nieodwiedzone = (int*)malloc(M * sizeof(int));
    int* lista_odwiedzonych = (int*)malloc(M * sizeof(int));

    for (int i = 0; i < N; i++) {
        if (swatka->para_w_wierszu  [i] != -1) continue;

        int liczba_nieodwiedzonych = M;
        int liczba_odwiedzonych = 0;

        for (int j = 0; j < M; j++) {
            luz[j] = INF;
            nieodwiedzone[j] = j;
        }

        int obecny_wiersz = i;
        int docelowa_kolumna = -1;

        while (1) {
            int min_luz = INF;
            int nastepna_kolumna = -1;
            int indeks_nieodwiedzonego = -1;

            int pot_obecny = potencjal_wiersza[obecny_wiersz];
            int* wiersz_macierzy = swatka->wspolrzedne_potencjalow[obecny_wiersz];

            for (int j = 0; j < liczba_nieodwiedzonych; j++) {
                int k = nieodwiedzone[j];
                int obecny_luz = luz[k];
                int koszt_przejscia = wiersz_macierzy[k] - pot_obecny - potencjal_kolumny[k];

                if (koszt_przejscia < obecny_luz) {
                    obecny_luz = koszt_przejscia;
                    luz[k] = koszt_przejscia;
                    skad_przyszlismy[k] = obecny_wiersz;
                }
                if (obecny_luz < min_luz) {
                    min_luz = obecny_luz;
                    nastepna_kolumna = k;
                    indeks_nieodwiedzonego = j;
                }
            }

            potencjal_wiersza[i] += min_luz;
            for (int j = 0; j < liczba_odwiedzonych; j++) {
                int k = lista_odwiedzonych[j];
                potencjal_kolumny[k] -= min_luz;
                potencjal_wiersza[swatka->para_w_kolumnie[k]] += min_luz;
            }
            for (int j = 0; j < liczba_nieodwiedzonych; j++) {
                int k = nieodwiedzone[j];
                luz[k] -= min_luz;
            }

            lista_odwiedzonych[liczba_odwiedzonych++] = nastepna_kolumna;
            nieodwiedzone[indeks_nieodwiedzonego] = nieodwiedzone[--liczba_nieodwiedzonych];

            if (swatka->para_w_kolumnie[nastepna_kolumna] == -1) {
                docelowa_kolumna = nastepna_kolumna;
                break;
            }
            obecny_wiersz = swatka->para_w_kolumnie[nastepna_kolumna];
        }

        while (docelowa_kolumna != -1) {
            int r = skad_przyszlismy[docelowa_kolumna];
            int poprzednia_kolumna = swatka->para_w_wierszu[r];
            swatka->para_w_kolumnie[docelowa_kolumna] = r;
            swatka->para_w_wierszu[r] = docelowa_kolumna;
            docelowa_kolumna = poprzednia_kolumna;
        }
    }

    free(potencjal_wiersza);
    free(potencjal_kolumny);
    free(luz);
    free(skad_przyszlismy);
    free(nieodwiedzone);
    free(lista_odwiedzonych);
}
void zwolnij_swatke(Swatka* swatka) {
    for (int i = 0; i < swatka->liczba_klientow; i++) {
        Relacja* k = swatka->preferencje_klientow[i];
        while (k != NULL) {
            Relacja* temp = k;
            k = k->nastepna;
            free(temp);
        }
    }
    free(swatka->preferencje_klientow);
    free(swatka->plcie);

    if (swatka->macierz_potencjalow) free(swatka->macierz_potencjalow);
    if (swatka->oryginalna_macierz_potencjalow) free(swatka->oryginalna_macierz_potencjalow);
    if (swatka->wspolrzedne_potencjalow) free(swatka->wspolrzedne_potencjalow);
    if (swatka->oryginalne_wspolrzedne_potencjaly) free(swatka->oryginalne_wspolrzedne_potencjaly);
    if (swatka->para_w_wierszu) free(swatka->para_w_wierszu);
    if (swatka->para_w_kolumnie) free(swatka->para_w_kolumnie);
}

long long oblicz_i_wypisz_wynik(Swatka* swatka) {
    long long wynik = 0;
    for (int j = 0; j < swatka->liczba_kolumn; j++) {
        if (swatka->para_w_kolumnie[j] != -1) {
            wynik += swatka->oryginalne_wspolrzedne_potencjaly[swatka->para_w_kolumnie[j]][j];
        }
    }
    printf("%lld\n", wynik);
    return wynik;
}
int main() {
    int liczba_testow;
    if (scanf("%d", &liczba_testow) != 1) return 0;

    for (int t = 0; t < liczba_testow; t++) {
        int n, m;
        scanf("%d %d", &n, &m);

        Swatka swatka;
        inicjalizuj_swatke(&swatka, n);

        for (int i = 0; i < m; i++) {
            int u, v, waga;
            scanf("%d %d %d", &u, &v, &waga);
            dodaj_relacje(&swatka, u - 1, v - 1, waga);
        }
        podziel_na_grupy(&swatka);
        zbuduj_macierz(&swatka);
        przygotuj_koszty_i_heurystyke(&swatka);
        szukaj_sciezki_wegierskiej(&swatka);
        oblicz_i_wypisz_wynik(&swatka);
        zwolnij_swatke(&swatka);
    }
    return 0;
}