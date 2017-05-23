#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define NUM_OF_LABELS 10

static int confusion_matrix[NUM_OF_LABELS][NUM_OF_LABELS];
static int num_of_characteristics;

// Calcula a distancia entre 2 pontos.
double calcdist(double *p1, double *p2) {
    double distsum = 0.0;
    int i;
    for(i = 0; i < (num_of_characteristics - num_of_characteristics%4); i += 4) {
        distsum += fabs(p1[i] * p1[i] - p2[i] * p2[i]);
        distsum += fabs(p1[i + 1] * p1[i + 1] - p2[i + 1] * p2[i + 1]);
        distsum += fabs(p1[i + 2] * p1[i + 2] - p2[i + 2] * p2[i + 2]);
        distsum += fabs(p1[i + 3] * p1[i + 3] - p2[i + 3] * p2[i + 3]);
    }
    for(;i < num_of_characteristics; i++) {
        distsum += fabs(p1[i] * p1[i] - p2[i] * p2[i]);
    }
    return distsum;
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        fprintf(stderr, "Uso: %s <k> <path to train data> <path to test data>\n", argv[0]);
        return 1;
    }
    char *end;
    int kval = strtol(argv[1], &end, 10);
    if(*end != '\0' || kval < 1) {
        fputs("ERRO: K inválido.\n", stderr);
        return 2;
    }
    printf("* K Nearest Neighbor - K = %d.\n", kval);
    printf("* Autor: Kaio Augusto de Camargo.\n\n");
    FILE *train_file = fopen(argv[2], "r"), *test_file = fopen(argv[3], "r");
    if(!train_file) {
        fputs("ERRO: Não foi possível abrir o arquivo de treinamento.\n", stderr);
        return 3;
    }
    if(!test_file) {
        fputs("ERRO: Não foi possível abrir o arquivo de teste.\n", stderr);
        return 4;
    }

    for(int i = 0; i < NUM_OF_LABELS; i++) {
        for(int j = 0; j < NUM_OF_LABELS; j++) {
            confusion_matrix[i][j] = 0;
        }
    }

    int num_of_train_data;
    fscanf(train_file, "%d %d", &num_of_train_data, &num_of_characteristics);
    double **train_data = (double **) malloc(sizeof(double *) * num_of_train_data);
    int *train_label = (int *) malloc(sizeof(int) * num_of_train_data);
    if(!train_data || !train_label) {
        fputs("ERRO: Não foi possível alocar espaço para os dados de treino!\n", stderr);
        return 5;
    }
    for(int i = 0; i < num_of_train_data; i++) {
        train_data[i] = (double *) malloc(sizeof(double) * num_of_characteristics);
        if(!train_data[i]) {
            fputs("ERRO: Não foi possível alocar espaço para os dados de treino.\n", stderr);
            return 6;
        }
        for(int j = 0; j < num_of_characteristics; j++) {
            fscanf(train_file, "%lf", train_data[i] + j);
        }
        fscanf(train_file, "%d", train_label + i);
    }
    printf("Treinamento concluído! Iniciando testes! (k = %d)\n", kval);
    
    int num_of_test_data, test_charac;
    fscanf(test_file, "%d %d", &num_of_test_data, &test_charac);
    if(test_charac != num_of_characteristics) {
        fputs("ERRO: O número de características do conjunto de testes é diferente do número de características do conjunto de treino!\n", stderr);
        printf("%d != %d!\n", test_charac, num_of_characteristics);
        return 7;
    }
    double *test_characteristiscs = (double *) malloc(sizeof(double) * num_of_characteristics), 
           *distance_to_closest = (double *) malloc(sizeof(double) * kval);
    int *closest_data = (int *) malloc(sizeof(int) * kval);
    if(!test_characteristiscs || !closest_data || !distance_to_closest) {
        fputs("ERRO: Não foi possível alocar espaço para os testes de teste!\n", stderr);
        return 8;
    }
    for(int i = 0; i < num_of_test_data; i++) {
        for(int j = 0; j < kval; j++) {
            closest_data[j] = -1;
        }
        for(int j = 0; j < num_of_characteristics; j++) {
            fscanf(test_file, "%lf", test_characteristiscs + j);
        }
        int real_label;
        fscanf(test_file, "%d", &real_label);
        for(int j = 0; j < num_of_train_data; j++) {
            double dist = calcdist(test_characteristiscs, train_data[j]);
            for(int k = 0; k < kval; k++) {
                if(closest_data[k] == -1) {
                    closest_data[k] = j;
                    distance_to_closest[k] = dist;
                    break;
                }
                if(dist < distance_to_closest[k]) {
                    for(int l = kval - 1; l > k; l--) {
                        closest_data[l] = closest_data[l-1];
                        distance_to_closest[l] = distance_to_closest[l-1];
                    }
                    closest_data[k] = j;
                    distance_to_closest[k] = dist;
                    break;
                }
            }
        }
        int *neighbor_labels = (int *) calloc(kval, sizeof(int));
        for(int j = 0; j < kval; j++) {
            neighbor_labels[train_label[closest_data[j]]]++;
        }
        int max_label = 0;
        for(int j = 1; j < NUM_OF_LABELS; j++) {
            if(neighbor_labels[max_label] < neighbor_labels[j]) max_label = j;
        }
        confusion_matrix[max_label][real_label]++;
        free(neighbor_labels);
    }
    printf("Fim do teste!\nMatriz de confusão:\n");
    printf("  %6d %6d %6d %6d %6d %6d %6d %6d %6d %6d\n", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9); 
    int acertos = 0;
    for(int i = 0; i < NUM_OF_LABELS; i++) {
        printf("%d ", i);
        for(int j = 0; j < NUM_OF_LABELS; j++) {
            printf("%6d ", confusion_matrix[i][j]);
            if(i == j) acertos += confusion_matrix[i][j];
        }
        printf("\n");
    }
    printf("\nAcertos: %d/%d (%lf%%).\n", acertos, num_of_test_data, ((double)acertos)/num_of_test_data);
}
