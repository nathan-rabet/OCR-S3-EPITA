#include "network.h"
#include "layer.h"
#include "neurone.h"

#include <json.h>
#include <stdlib.h>
#include <stdio.h>

struct Network CreateNetwork(struct Layer *layers,size_t nb_layers) {
    struct Network network;
    network.layers = layers;
    network.nb_layers = nb_layers;
    return network;
}

void appendFirstLayerToNetwork(struct Network *network) {
    struct Neurone *firstNeurones = malloc(sizeof(struct Neurone) * network->layers->neurones->nb_inputs);
    double *uniqueWeight = malloc(sizeof(double));
    *uniqueWeight = 1;
    for (size_t i = 0; i < network->layers->neurones->nb_inputs; i++)
    {
        firstNeurones[i] = CreateNeurone(uniqueWeight,0,0,1);
        firstNeurones[i].nextNeuroneSameLayer = &firstNeurones[i+1];
    }

    firstNeurones[network->layers->neurones->nb_inputs - 1].nextNeuroneSameLayer = NULL;

    struct Layer *firstLayer = malloc(sizeof(struct Layer)); 
    *firstLayer = CreateLayer(firstNeurones,network->layers->neurones->nb_inputs);
    firstLayer->nextLayer = network->layers;
    network->layers = firstLayer;
}

struct Network LoadNetworkFromJSON(char jsonFilePath[]) {    
    struct Network network;

    char *fileData;
    FILE *f = fopen (jsonFilePath, "rb");

    if (f)
    {
        long length;

        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        fileData = malloc(length);
        if (fileData)
        {
            fread(fileData, 1, length, f);
        }
        fclose (f);
    }

    if (fileData)
    {
        struct json_object *parsed_json = json_tokener_parse(fileData);
        free(fileData);

        // Point the layers
        struct json_object *JSONnb_layers = NULL;
        struct json_object *JSONlayers = NULL;

        json_object_object_get_ex(parsed_json,"nb_layers",&JSONnb_layers);
        json_object_object_get_ex(parsed_json,"layers",&JSONlayers);

        network.nb_layers = json_object_get_int(JSONnb_layers);
        
        // ? Layers
		struct Layer *layers = NULL;
        layers = malloc(sizeof(struct Layer) * network.nb_layers);
        
        for (size_t l = 0; l < network.nb_layers; l++)
        {
            // Point to a layer l of the network
            struct json_object *JSONlayer = json_object_array_get_idx(JSONlayers,l);

            struct json_object *JSONnb_neurones;
            json_object_object_get_ex(JSONlayer,"nb_neurones",&JSONnb_neurones);

            size_t nb_neurones = (size_t) json_object_get_int(JSONnb_neurones);


            struct json_object *JSONneurones;
            json_object_object_get_ex(JSONlayer,"neurones",&JSONneurones);

            // ? Neurones
            struct Neurone *neurones = NULL;
            neurones = malloc(sizeof(struct Neurone) * nb_neurones);
            for (size_t n = 0; n < nb_neurones; n++)
            {
                struct json_object *JSONbias = NULL;
                struct json_object *JSONactivationFunction = NULL;
                struct json_object *JSONnb_weights = NULL;
                struct json_object *JSONweights = NULL;
                struct json_object *JSONneurone = NULL;

                // Point to the neurone n of the layer l
                JSONneurone = json_object_array_get_idx(JSONneurones,n);

                json_object_object_get_ex(JSONneurone,"bias",&JSONbias);
                json_object_object_get_ex(JSONneurone,"activationFunction",&JSONactivationFunction);
                json_object_object_get_ex(JSONneurone,"nb_weights",&JSONnb_weights);
                json_object_object_get_ex(JSONneurone,"weights",&JSONweights);

                double bias = json_object_get_double(JSONbias);
                unsigned char activationFunction = (unsigned char) json_object_get_int(JSONactivationFunction);
                size_t nb_weights = (size_t) json_object_get_int(JSONnb_weights);

                double *weights = malloc(sizeof(double) * nb_weights);
                for (size_t w = 0; w < nb_weights; w++)
                {
                    struct json_object *JSONweight = json_object_array_get_idx(JSONweights,w);
                    weights[w] = json_object_get_double(JSONweight);
                }

                neurones[n] = CreateNeurone(weights,bias,activationFunction,nb_weights);
                
                // Neurones linking
                if (n >= 1) {
                    neurones[n-1].nextNeuroneSameLayer = &neurones[n];
                }
            }

            neurones[nb_neurones - 1].nextNeuroneSameLayer = NULL;

            layers[l] = CreateLayer(neurones,nb_neurones);
            
            // Layers linking
            if (l >= 1) {
                layers[l-1].nextLayer = &layers[l];
            }
        }
        layers[network.nb_layers - 1].nextLayer = NULL;

        network.layers = layers;
    }

    else {
        fprintf(stderr, "The file '%s' doesn't exist.", jsonFilePath);
    }

    appendFirstLayerToNetwork(&network);
    
    return network;
}

size_t networkNbInput(struct Network network) {
    return network.layers[0].nb_neurones;
}

size_t networkNbOutput(struct Network network) {
    
    size_t nb_output = 0;

    struct Layer *workingLayer = network.layers;

    while (workingLayer != NULL) {
        nb_output = workingLayer->nb_neurones;
        workingLayer = workingLayer->nextLayer;
    }
    
    return nb_output;
}

size_t networkNbNeurones(struct Network network) {
    
    size_t nb_neurones = 0;

    struct Layer *workingLayer = network.layers;

    while (workingLayer != NULL) {
        nb_neurones += workingLayer->nb_neurones;
        workingLayer = workingLayer->nextLayer;
    }
    
    return nb_neurones;
}

size_t networkNbWeights(struct Network network) {
    
    size_t nb_weights = 0;

    struct Layer *workingLayer = network.layers;

    while (workingLayer != NULL) {
        size_t weights = workingLayer->nb_neurones;
        workingLayer = workingLayer->nextLayer;
        nb_weights += weights * workingLayer->nb_neurones;
    }
    
    return nb_weights;
}

double * calculateNetworkOutput(struct Network network, double input[]) {
    double *nextInput = input;
    double *outputNetwork;
    
    struct Layer *workingLayer = network.layers;

    // Entry
    outputNetwork = malloc(sizeof(double) * (workingLayer->nb_neurones));
    for (size_t i = 0; i < workingLayer->nb_neurones; i++)
    {
        double *entry = &input[i];
        outputNetwork[i] = calculateNeuroneOutput(workingLayer->neurones[i],entry);
    }
    workingLayer = workingLayer->nextLayer;
    nextInput = outputNetwork;

    // General case
    while(workingLayer != NULL) {
        outputNetwork = malloc(sizeof(double) * (workingLayer->nb_neurones));
        outputNetwork = CalculateLayerOutput(*workingLayer, nextInput);

        nextInput = outputNetwork;
        workingLayer = workingLayer->nextLayer;
    }

    return outputNetwork;
} 