mat4 QuaternionToMat4(vec4 q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    mat4 result;

    result[0][0] = 1.0 - 2.0 * (y2 + z2);
    result[0][1] = 2.0 * (xy - wz);
    result[0][2] = 2.0 * (xz + wy);
    result[0][3] = 0.0;

    result[1][0] = 2.0 * (xy + wz);
    result[1][1] = 1.0 - 2.0 * (x2 + z2);
    result[1][2] = 2.0 * (yz - wx);
    result[1][3] = 0.0;

    result[2][0] = 2.0 * (xz - wy);
    result[2][1] = 2.0 * (yz + wx);
    result[2][2] = 1.0 - 2.0 * (x2 + y2);
    result[2][3] = 0.0;

    result[3][0] = 0.0;
    result[3][1] = 0.0;
    result[3][2] = 0.0;
    result[3][3] = 1.0;

    return result;
}
