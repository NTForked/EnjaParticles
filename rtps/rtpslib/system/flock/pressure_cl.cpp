#include "cl_structs.h"

float magnitude(float4 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}
float dist_squared(float4 vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

__kernel void pressure(__global float4* pos, __global float* density, __global float4* force, __constant struct FLOCKParams* params)
{
    unsigned int i = get_global_id(0);
    int num = params->num;
    if(i > num) return;


    float4 f = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

    float4 p = pos[i] * params->simulation_scale;
    float di = density[i];

    float h = params->smoothing_distance;

    //stuff from Tim's code (need to match #s to papers)
    //float alpha = 315.f/208.f/params->PI/h/h/h;
    float h6 = h*h*h * h*h*h;
    float alpha = -45.f/params->PI/h6;


    //super slow way, we need to use grid + sort method to get nearest neighbors
    //this code should never see the light of day on a GPU... just sayin
    for(int j = 0; j < num; j++)
    {
        if(j == i) continue;
        float4 pj = pos[j] * params->simulation_scale;
        float4 r = p - pj;
        float rlen = magnitude(r);
        if(rlen < h)
        {
            float r2 = rlen*rlen;
            float re2 = h*h;
            if(r2/re2 <= 4.f)
            {
                //float R = sqrt(r2/re2);
                //float Wij = alpha*(-2.25f + 2.375f*R - .625f*R*R);
                float hr2 = (h - rlen);
                float Wij = alpha * hr2*hr2/rlen;
                //from tim's code
                //float Pi = 1.013E5*(pow(density[i]/1000.0f, 7.0f) - 1.0f);
                //float Pj = 1.013E5*(pow(density[j]/1000.0f, 7.0f) - 1.0f);
                //float kern = params->mass * Wij * (Pi + Pj) / (density[i] * density[j]);
                
                float dj = density[j];
                //form simple FLOCK in Krog's thesis
                float Pi = params->K*(di - 1000.0f); //rest density
                float Pj = params->K*(dj - 1000.0f); //rest density
                //float kern = params->mass * -1.0f * Wij * (Pi + Pj) / (2.0f * density[j]);
                //float kern = params->mass * -1.0f * Wij * (Pi + Pj) / (density[i] * density[j]);
                float kern = params->mass * -.5f * Wij * (Pi + Pj) / (di * dj);
                //float kern = params->mass * -1.0f * Wij * (Pi/(di*di) + Pj/(dj*dj));
                f += kern * r;
            }

        }
    }
    force[i] = f;
}