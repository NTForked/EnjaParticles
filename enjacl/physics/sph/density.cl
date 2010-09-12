#define STRINGIFY(A) #A

//update the SPH density
std::string density_program_source = STRINGIFY(

float magnitude(float4 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

__kernel void density(__global float4* pos, __global float* density)
{
    unsigned int i = get_global_id(0);
    //obviously this is going to be passed in as a parameter
    int num = 1024;
    float rest_distance = 0.025641f;
    float smoothing_length = 2.0f * rest_distance;

    //stuff from Tim's code (need to match #s to papers)
    float pi = 4.f * atan(1.0f);
    float alpha = 315.f/208.f/pi/smoothing_length/smoothing_length/smoothing_length;
    float m = 1.0f; //mass = 1 ??

    float4 p = pos[i];

    //super slow way, we need to use grid + sort method to get nearest neighbors
    //this code should never see the light of day on a GPU... just sayin
    for(int j = 0; j < num; j++)
    {
        float4 r = p - pos[j];
        float rlen = magnitude(r);
        if(rlen < smoothing_length)
        {
            //float q = 1.0f - rlen / smoothing_length;
            float r2 = rlen*rlen;
            float re2 = smoothing_length*smoothing_length;
            if(r2/re2 <= 4.f)
            {
                float R = sqrt(r2/re2);
                float Wij = alpha*(2.f/3.f - 9.f*R*R/8.f + 19.f*R*R*R/24.f - 5.f*R*R*R*R/32.f);
                density[i] += m * Wij;
            }
        }
    }

}
);

