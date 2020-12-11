#include "backpropagation.h"
#include "../struct/networks.h"
#include "../struct/network.h"

#include "../../miscellaneous/color.h"

#define LEARNINGRATE 0.01
#define MINIBATCH_SIZE 100
#define NB_TRAINING_PER_MINIBATCH 9
#define NB_MINIBATCH 1000000
// 1/30

double cost(struct Network *network, size_t expected_outputs_index)
{
    // cost = ½∑(a-y)²

    double sum = 0;
    for (size_t i = 0; i < networkNbOutput(network); i++)
    {
        double o = network->layers[network->nb_layers - 1]->output[i];
        sum += pow(o - (i == expected_outputs_index), 2);
    }

    sum /= 2;
    return sum;
}

double cost_derivate(struct Network *network, double *expected_outputs)
{
    // cost' = ∑a-y

    double sum = 0;
    for (size_t i = 0; i < networkNbOutput(network); i++)
    {
        double o_i = network->layers[network->nb_layers - 1]->output[i];
        sum += o_i - expected_outputs[i];
    }
    return sum;
}

void train(struct Networks *networks, char *datasetpath)
{
    for (size_t net = 0; net < networks->nb_networks; net++)
    {
        struct Network *network = networks->networks[net];
        //Initializing neurones training parameters
        for (size_t l = 1; l < network->nb_layers; l++)
        {
            for (size_t n = 0; n < network->layers[l]->nb_neurones; n++)
            {
                if ((network->layers[l]->neurones[n]->delta_bias) != NULL && network->layers[l]->neurones[n]->delta_weight != NULL)
                {
                    free(network->layers[l]->neurones[n]->delta_bias);
                    free(network->layers[l]->neurones[n]->delta_weight);
                }
                network->layers[l]->neurones[n]->delta_bias = malloc(MINIBATCH_SIZE * sizeof(double));
                network->layers[l]->neurones[n]->delta_weight = malloc(network->layers[l - 1]->nb_neurones * MINIBATCH_SIZE * sizeof(double));
            }
        }

        // Create NB_MINIBATCH minibatches
        for (size_t b = 0; b < NB_MINIBATCH; b++)
        {
            double **expected_output = malloc(MINIBATCH_SIZE * sizeof(double *));
            double **inputs = malloc(MINIBATCH_SIZE * sizeof(double *));

            for (size_t i = 0; i < MINIBATCH_SIZE; i++)
            {
                // Define one minibatch size to MINIBATCH_SIZE
                expected_output[i] = calloc(networkNbOutput(network), sizeof(double)); // expected_output[i] -> Target vector : (1 0 0 0 1 0 0 1 0 0 0)
                /* 
                    /!\ 
                    Implementation for charcters only
                */
                int char_index_list = rand() % CHARSLEN;

                // Check if the random letter is the one managed by the network
                if (CHARS[char_index_list] == network->character)
                    expected_output[i][0] = 1;

                double *tamerelapute = loadDataBase(datasetpath, CHARS[char_index_list], (rand() % 1000) + 1);
                inputs[i] = tamerelapute;
            }

            printf("MINI-BATCH n°%lu\n", b);

            for (size_t i = 0; i < NB_TRAINING_PER_MINIBATCH; i++)
            {
                minibatch(network, inputs, expected_output);
            }

            // Free inputs & and expected ones
            for (size_t i = 0; i < MINIBATCH_SIZE; i++)
                free(inputs[i]);
            for (size_t i = 0; i < MINIBATCH_SIZE; i++)
                free(expected_output[i]);
            // dadim dam dam dadim dam dam dadadi dadadadi do dim dam dam

            CalculateScore(network, datasetpath);
            printf("\n");
        }
    }
}

void minibatch(struct Network *network, double **inputs, double **expected_output)
{
    for (size_t i = 0; i < MINIBATCH_SIZE; i++)
    {
        // Define minibatch
        double *input = inputs[i];

        // Feedforward (run the network with input to set the z and activation values)
        calculateNetworkOutput(network, input);

        // Backpropagate the error
        backpropagation(network, expected_output[i]);

        // Set ∂cost
        for (size_t l = 1; l < network->nb_layers; l++)
        {
            for (size_t n = 0; n < network->layers[l]->nb_neurones; n++)
            {
                struct Neurone *neurone = (network->layers[l]->neurones[n]);

                // ∂cost/∂b_i = δl_i
                neurone->delta_bias[i] = neurone->delta_error;

                for (size_t k = 0; k < network->layers[l - 1]->nb_neurones; k++)
                {
                    // ∂cost/∂w_ij = a_j * δl_i
                    // i : image i
                    // k : data of the image i
                    neurone->delta_weight[k * MINIBATCH_SIZE + i] = activationFunction((network->layers[l - 1]->neurones[k])) * neurone->delta_error;
                }
            }
        }
    }

    // Update ∂bias and ∂weight
    for (size_t l = 1; l < network->nb_layers; l++)
    {
        for (size_t n = 0; n < network->layers[l]->nb_neurones; n++)
        {
            struct Neurone *neurone = (network->layers[l]->neurones[n]);

            for (size_t k = 0; k < network->layers[l]->neurones[0]->nb_inputs; k++)
            {
                double sumweights = 0;

                for (size_t i = 0; i < MINIBATCH_SIZE; i++)
                {

                    sumweights += neurone->delta_weight[k * MINIBATCH_SIZE + i];
                }

                neurone->weights[k] -= (LEARNINGRATE) * (sumweights / MINIBATCH_SIZE);
            }

            double sum_delta_bias = 0;
            for (size_t i = 0; i < MINIBATCH_SIZE; i++)
            {
                sum_delta_bias += neurone->delta_bias[i];
            }
            neurone->bias -= (LEARNINGRATE) * (sum_delta_bias / MINIBATCH_SIZE);
        }
    }
}

void backpropagation(struct Network *network, double *expected_output)
{
    // δL = cost'(a) * actvation_fonction'(z)
    for (size_t k = 0; k < networkNbOutput(network); k++)
    {
        struct Neurone *nk = (network->layers[network->nb_layers - 1]->neurones[k]);
        nk->delta_error = cost_derivate(network, expected_output) * actvation_fonction_derivate(nk);
    }

    // δl_i = actvation_fonction'(z) * ∑(w_ji * δ_j)
    //                               j
    // l=L−1,L−2,…,2

    //        *  *                         *  *
    //     *        *         00        *        *
    //    *    i0    *    --------->   *    j0    *
    //    *          *                 *          *
    //     *        *    \              *        *
    //        *  *         \ 10            *  *        | δl_i = actvation_fonction'(z) * ∑(w_ji * δ_j)
    //                       \                         |                                 j
    //        *  *             \           *  *        | l=L−1,L−2,…,2
    //     *        *            \      *        *
    //    *    i1    *             \   *    j1    *
    //    *          *              -- *          *
    //     *        *                   *        *
    //        *  *                         *  *

    for (size_t l = network->nb_layers - 2; l > 0; l--)
    {
        for (size_t i = 0; i < network->layers[l]->nb_neurones; i++)
        {
            double sum = 0;
            for (size_t j = 0; j < network->layers[l + 1]->nb_neurones; j++)
            {
                // neurones[j] -> change | weights[i] don't change
                sum += network->layers[l + 1]->neurones[j]->delta_error * network->layers[l + 1]->neurones[j]->weights[i];
            }
            network->layers[l]->neurones[i]->delta_error = sum * actvation_fonction_derivate(network->layers[l]->neurones[i]);
        }
    }
}

void CalculateScore(struct Network *network, char *databasepath)
{
    printf("SCORE : network '%c'\n", network->character);
    int number_of_test = 1000;
    int nb_success = 0;

    double cost_average = 0;
    for (int i = 0; i < number_of_test; i++)
    {
        char letter = CHARS[rand() % CHARSLEN];

        // Dataset loading
        double *inputs = loadDataBase(databasepath, letter, (rand() % 1000) + 1);

        // Feedforward
        double *outputs = calculateNetworkOutput(network, inputs);

        if (letter == network->character)
        {
            if (*outputs > 0.8)
                nb_success += 1;
        }
        else
        {
            if (*outputs < 0.5)
                nb_success += 1;
        }

        cost_average += cost(network, letter == network->character ? 0 : 42);

        free(outputs);
        free(inputs);
    }

    cost_average /= number_of_test;
    float percentage_of_success = ((100 * nb_success) / number_of_test);
    char *color;

    if (percentage_of_success > 90)
        color = GRN;

    else if (percentage_of_success > 50)
        color = YEL;
    else
        color = RED;

    printf("COST : %f\n", cost_average);
    printf("PERCENTAGE : %s%f%%%s. [%d/%d]\n", color, percentage_of_success, RST, nb_success, number_of_test);
}

double *loadDataBase(char *databasepath, char letter, size_t imagenumber)
{
    //Convert a imagenumber to a "12345" string
    char *imagename = malloc(6 * sizeof(char));
    imagename[5] = 0;
    for (size_t i = 5; i > 0; i--)
    {
        imagename[i - 1] = '0' + imagenumber % 10;
        imagenumber /= 10;
    }

    //Build the image path string
    char *imagepath = malloc((strlen(databasepath) + 150) * sizeof(char));
    sprintf(imagepath, "%s/%02x/train_%02x/train_%02x_%s.png", databasepath, letter, letter, letter, imagename);

    //Load image
    SDL_Surface *image = loadImage(imagepath);
    free(imagepath);
    free(imagename);

    double *imagebin = binarizationpointer(image, 8);
    SDL_FreeSurface(image);
    return imagebin;
}

void PrintInput(double *input, size_t height, size_t with)
{
    printf("Input:\n");
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < with; j++)
        {
            if (*(input + i * with + j) > 0.5)
                printf("%i", 1);
            else
                printf(" ");
        }
        printf("|\n");
    }
    scanf("\n");
}

void PrintOuput(double *output, char letter)
{
    printf("Output: { ");
    printf("%c=%f", letter, *output);
    printf(" }\n");
}