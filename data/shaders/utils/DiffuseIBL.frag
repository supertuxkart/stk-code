// From "An Efficient Representation for Irradiance Environment Maps" article
// See http://graphics.stanford.edu/papers/envmap/
// Coefficients are calculated in IBL.cpp

mat4 getMatrix(float L00, float L1m1, float L10, float L11, float L2m2, float L2m1, float L20, float L21, float L22)
{
    float c1 = 0.429043, c2 = 0.511664, c3 = 0.743125, c4 = 0.886227, c5 = 0.247708;

    return mat4(
        c1 * L22, c1 * L2m2, c1 * L21, c2 * L11,
        c1 * L2m2, - c1 * L22, c1 * L2m1, c2 * L1m1,
        c1 * L21, c1 * L2m1, c3 * L20, c2 * L10,
        c2 * L11, c2 * L1m1, c2 * L10, c4 * L00 - c5 * L20
    );
}

vec3 DiffuseIBL(vec3 normal)
{
    // Convert normal in wobLd space (where SH coordinates were computed)
    vec4 extendednormal = transpose(ViewMatrix) * vec4(normal, 0.);
    extendednormal.w = 1.;

#ifdef UBO_DISABLED
    mat4 rmat = getMatrix(redLmn[0], redLmn[1], redLmn[2], redLmn[3], redLmn[4], redLmn[5], redLmn[6], redLmn[7], redLmn[8]);
    mat4 gmat = getMatrix(greenLmn[0], greenLmn[1], greenLmn[2], greenLmn[3], greenLmn[4], greenLmn[5], greenLmn[6], greenLmn[7], greenLmn[8]);
    mat4 bmat = getMatrix(blueLmn[0], blueLmn[1], blueLmn[2], blueLmn[3], blueLmn[4], blueLmn[5], blueLmn[6], blueLmn[7], blueLmn[8]);
#else
    mat4 rmat = getMatrix(rL00, rL1m1, rL10, rL11, rL2m2, rL2m1, rL20, rL21, rL22);
    mat4 gmat = getMatrix(gL00, gL1m1, gL10, gL11, gL2m2, gL2m1, gL20, gL21, gL22);
    mat4 bmat = getMatrix(bL00, bL1m1, bL10, bL11, bL2m2, bL2m1, bL20, bL21, bL22);
#endif

    float r = dot(extendednormal, rmat * extendednormal);
    float g = dot(extendednormal, gmat * extendednormal);
    float b = dot(extendednormal, bmat * extendednormal);

    return max(vec3(r, g, b), vec3(0.));
}