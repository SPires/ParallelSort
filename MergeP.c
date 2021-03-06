#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

void imprimir_vetor(int* vetor, int tamanho);

void merge(int* vetor1, int tamanho1, int* vetor2, int tamanho2) {
  int i = 0, indice_um = 0, indice_dois = 0;
  int tamanho = tamanho1 + tamanho2;
  int resposta[tamanho];

  while(indice_um < tamanho1 && indice_dois < tamanho2) {
    if(vetor1[indice_um] < vetor2[indice_dois])
      resposta[i] = vetor1[indice_um++];
    else
      resposta[i] = vetor2[indice_dois++];
    i++;
  }

  while(indice_um < tamanho1)
    resposta[i++] = vetor1[indice_um++];
  while(indice_dois < tamanho2)
    resposta[i++] = vetor2[indice_dois++];


  for(i = 0; i < tamanho1; i++)
    vetor1[i] = resposta[i];

  for(i = 0; i < tamanho2; i++)
    vetor2[i] = resposta[i + tamanho1];

}

void _merge_sort(int* vetor, int tamanho) {
  if(tamanho == 1) return;
  else{
    int meio = (tamanho)/2;
    int *metade1_elemento = vetor;
    int metade1_tamanho = meio;

    int *metade2_elemento = &(vetor[meio]);
    int metade2_tamanho = tamanho - meio;

    _merge_sort(metade1_elemento, metade1_tamanho);
    _merge_sort(metade2_elemento, metade2_tamanho);

    merge(metade1_elemento, metade1_tamanho, metade2_elemento, metade2_tamanho);
  }
}

void imprimir_vetor(int* vetor, int tamanho) {
  int i;
  printf("[");
  for (i = 0; i < tamanho; i++)
    if(i == tamanho -1)
      printf("%d]", vetor[i]);
    else
      printf("%d, ", vetor[i]);

  printf("\n");
}

int esta_correto(int* vetor, int tamanho) {
  int i;
  for (i = 0; i < tamanho-1; i++)
    if (vetor[i] > vetor[i+1]) return 0;

  return 1;
}

void concatenar_vetores(int* vetor1, int tamanho1, int* vetor2, int tamanho2, int* resposta){
  int i, j;
  for(i = 0; i < tamanho1; i++){
    resposta[i] = vetor1[i];
  }
  for(j=0; j < tamanho2; j++){
    resposta[i+j] = vetor2[j];
  }
}

int main(int argc,char** argv){
  int my_rank;
  int numero_processos;   // número de processos
  int dest = 0;
  int tag=200;            // tipo de mensagem (único)
  int *vetor_local;       // endereco inicial do vetor local
  int indice_local;       // indice inicial o merge
  int tamanho_local;      // numero de elementos a ordenar
  int tamanho_leitura = 20;
  int leitura[20] = {5, 3, 4, 1, 0, 2, 6, 7, 9, 11, 10, 15, 13, 12, 8, 14, 19, 17, 18, 16};
  double start_time, end_time;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numero_processos);

  if(my_rank == 0)
  	start_time = MPI_Wtime();
  
  tamanho_local = tamanho_leitura/numero_processos;
  indice_local = tamanho_local * my_rank;
  vetor_local = leitura + indice_local;
  
  _merge_sort(vetor_local, tamanho_local);  

  if(my_rank == 0){
    int i;
    int recv[tamanho_local], ordenado[tamanho_leitura];
    concatenar_vetores(vetor_local, tamanho_local, ordenado, 0, ordenado);
    for (i = 1; i < numero_processos; i++){
      MPI_Recv(recv, tamanho_local, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
      merge(ordenado, tamanho_local*i, recv, tamanho_local);
      concatenar_vetores(ordenado, tamanho_local*i, recv, tamanho_local, ordenado);
    }
    if (tamanho_leitura%numero_processos != 0){
      int *final = &leitura[tamanho_local*numero_processos];
      int tamanho_final = tamanho_leitura%numero_processos;  
      _merge_sort(final, tamanho_final);
      merge(ordenado, tamanho_leitura - tamanho_final, final, tamanho_final);
      concatenar_vetores(ordenado, tamanho_local*i, final, tamanho_final, ordenado);
    }
    printf("\nOrdenado:");
    imprimir_vetor(ordenado, tamanho_leitura);
  }
  else{
    MPI_Send(vetor_local, tamanho_local, MPI_INT, dest, tag, MPI_COMM_WORLD);
  }

  if(my_rank == 0){
  	end_time = MPI_Wtime();
  	printf("\nTempo total de execução: %f.\n", end_time-start_time);
  }
  MPI_Finalize();
  return 0;
}
