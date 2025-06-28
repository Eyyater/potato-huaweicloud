// 通用归一化函数：将 input[0..n-1] 归一化到 output[0..n-1]
void normalize_array(const float *input, float *output, int n) {
    // 1. 找到最小值和最大值
    float vmin = input[0], vmax = input[0];
    for (int i = 1; i < n; ++i)
    {
        if (input[i] < vmin)
            vmin = input[i];
        if (input[i] > vmax)
            vmax = input[i];
    }
    // 2. 计算极差，避免除以零
    float vrange = vmax - vmin;
    if (vrange == 0.0f)
        vrange = 1.0f;

    // 3. 应用 (x - min) / (max - min)
    for (int i = 0; i < n; ++i)
    {
        output[i] = (input[i] - vmin) / vrange;
    }
}

float predict_data(const float *k, float b, const float *vis, const float *ir)
{ // 计算预测数据
    float y[12];
    for (int i = 0; i < 6; ++i)
    {
        y[i] = k[i] * vis[i] + b;
    }
    for (int i = 6; i < 12; ++i)
    {
        y[i] = k[i] * ir[i - 6] + b;
    }
    float sum = 0.0f;
    for (int i = 0; i < 12; i++)
    {
        sum += y[i];
    }
    return sum;
}
