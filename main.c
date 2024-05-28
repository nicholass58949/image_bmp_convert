#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// BMP文件头结构
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

// 从32位BMP转换为24位BMP
void convert32To24Bit(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    if (!inputFile) {
        perror("Error opening input file");
        return;
    }

    // 读取BMP文件头
    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, inputFile);

    // 读取BMP信息头
    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, inputFile);

    // 确保这是一个32位的BMP文件
    if (infoHeader.biBitCount != 32) {
        fprintf(stderr, "Input file is not a 32-bit BMP file.\n");
        fclose(inputFile);
        return;
    }

    // 修改信息头以表示24位图像
    infoHeader.biBitCount = 24;
    infoHeader.biSizeImage = ((infoHeader.biWidth * 3 + 3) & ~3) * abs(infoHeader.biHeight);
    // fileHeader.bfSize = 54 + infoHeader.biSizeImage;
    // 写入BMP文件头
    FILE *outputFile = fopen(outputPath, "wb");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(inputFile);
        return;
    }
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, outputFile);

    // 写入BMP信息头
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, outputFile);

    // 计算原始图像行的填充字节数
    int originalPadding = (4 - (infoHeader.biWidth * 4) % 4) % 4;

    // 计算新图像行的填充字节数
    int newPadding = (4 - (infoHeader.biWidth * 3) % 4) % 4;

    // 读取和转换每个像素
    for (int y = 0; y < abs(infoHeader.biHeight); y++) {
        for (int x = 0; x < infoHeader.biWidth; x++) {
            uint8_t rgba[4];
            fread(rgba, sizeof(uint8_t), 4, inputFile);

            // 写入24位的RGB数据，丢弃Alpha通道
            fwrite(rgba, sizeof(uint8_t), 3, outputFile);
        }
        // 跳过原始图像的填充字节
        fseek(inputFile, originalPadding, SEEK_CUR);
        // 写入新的填充字节
        for (int p = 0; p < newPadding; p++) {
            uint8_t paddingByte = 0x00;
            fwrite(&paddingByte, sizeof(uint8_t), 1, outputFile);
        }
    }

    fclose(inputFile);
    fclose(outputFile);
}

void convert24To18Bit(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    if (!inputFile) {
        perror("Error opening input file");
        return;
    }

    // 读取BMP文件头
    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, inputFile);

    // 读取BMP信息头
    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, inputFile);

    // 确保这是一个24位的BMP文件
    if (infoHeader.biBitCount != 24) {
        fprintf(stderr, "Input file is not a 24-bit BMP file.\n");
        fclose(inputFile);
        return;
    }

    // 修改信息头以表示18位图像
    uint32_t rowSize = (infoHeader.biWidth * 3 + 3) & (~3); // Align to 4 bytes
    uint32_t dataSize = rowSize * infoHeader.biHeight;

    fileHeader.bfSize = 54 + dataSize;
    // infoHeader.biBitCount = 18;
    infoHeader.biBitCount = 24;
    infoHeader.biSizeImage = dataSize;
    // infoHeader.biSizeImage = ((infoHeader.biWidth * 18 + 31) / 32) * 4 * abs(infoHeader.biHeight);

    printf("biSizeImage: %d\n", infoHeader.biSizeImage);
    printf("biWidth: %d\n", infoHeader.biWidth);
    printf("biHeight: %d\n", infoHeader.biHeight);

    // 创建输出文件
    FILE *outputFile = fopen(outputPath, "wb");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(inputFile);
        return;
    }

    // 写入BMP文件头
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, outputFile);

    // 写入BMP信息头
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, outputFile);

    // 计算图像行的填充字节数
    // int padding = (4 - (infoHeader.biWidth * 3) % 4) % 4;
    int padding = (4 - (infoHeader.biWidth * 3) % 4) % 4;
    // 处理每个像素
    for (int y = 0; y < abs(infoHeader.biHeight); y++)
    {
        for (int x = 0; x < infoHeader.biWidth; x++)
        {
            uint8_t rgb[3];
            fread(rgb, sizeof(uint8_t), 3, inputFile);

            // 将24位颜色转换为18位颜色
//            uint8_t r = rgb[2] >> 2; // 提取高6位
//            uint8_t g = rgb[1] >> 2;
//            uint8_t b = rgb[0] >> 2;

            uint8_t r = rgb[2] & 0xfc; // 提取高6位
            uint8_t g = rgb[1] & 0xfc;
            uint8_t b = rgb[0] & 0xfc;

            // 将6位颜色合并成18位数据 (占3字节)
            uint8_t rgb18[3] = {r, g, b};

            // 写入新的18位颜色数据
            fwrite(rgb18, sizeof(uint8_t), 3, outputFile);
        }
        // 跳过原始图像的填充字节
        fseek(inputFile, padding, SEEK_CUR);
        uint8_t zero = 0;
        fwrite(&zero, sizeof(uint8_t), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}

void convert24To16Bit(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    if (!inputFile) {
        perror("Error opening input file");
        return;
    }

    // 读取BMP文件头
    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, inputFile);

    // 读取BMP信息头
    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, inputFile);

    // 确保这是一个24位的BMP文件
    if (infoHeader.biBitCount != 24) {
        fprintf(stderr, "Input file is not a 24-bit BMP file.\n");
        fclose(inputFile);
        return;
    }

    // 修改信息头以表示18位图像
    uint32_t rowSize = (infoHeader.biWidth * 2 + 2) & (~3); // Align to 4 bytes
    uint32_t dataSize = rowSize * infoHeader.biHeight;

    fileHeader.bfSize = 54 + dataSize;
    // infoHeader.biBitCount = 18;
    infoHeader.biBitCount = 16;
    infoHeader.biSizeImage = dataSize;
    // infoHeader.biSizeImage = ((infoHeader.biWidth * 18 + 31) / 32) * 4 * abs(infoHeader.biHeight);

    printf("biSizeImage: %d\n", infoHeader.biSizeImage);
    printf("biWidth: %d\n", infoHeader.biWidth);
    printf("biHeight: %d\n", infoHeader.biHeight);

    // 创建输出文件
    FILE *outputFile = fopen(outputPath, "wb");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(inputFile);
        return;
    }

    // 写入BMP文件头
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, outputFile);

    // 写入BMP信息头
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, outputFile);

    // 计算图像行的填充字节数
    int padding_24bit = (4 - (infoHeader.biWidth * 3) % 4) % 4;
    int padding_16bit = (4 - (infoHeader.biWidth * 2) % 4) % 4;
    // 处理每个像素
    for (int y = 0; y < abs(infoHeader.biHeight); y++)
    {
        for (int x = 0; x < infoHeader.biWidth; x++)
        {
            uint8_t rgb[3];
            fread(rgb, sizeof(uint8_t), 3, inputFile);

//            // 将6位颜色合并成18位数据 (占3字节)
//            uint8_t rgb16_low = ((rgb[0] >> 3) & (0x1f)) | (rgb[1] << 3) & (0xe0);
//            uint8_t rgb16_high = ((rgb[1] >> 5) & (0x07)) | ((rgb[2] >> 0) | (0xf8));
//
//            // uint16_t rgb16 = (rgb[0] >> 3) | (rgb[1] >> 5) | (rgb[1] << 3) | (rgb[2] >> 3);
//            // uint16_t rgb16 = (rgb16_high << 8) | rgb16_low;
//            uint16_t rgb16 = (rgb16_high << 8) | rgb16_low;// 将24位颜色转换为16位颜色

            uint8_t r = rgb[2] >> 3; // 提取高5位
            uint8_t g = rgb[1] >> 3; // 提取高6位
            uint8_t b = rgb[0] >> 3; // 提取高5位

            // 将5位红色，6位绿色和5位蓝色合并成16位数据 (占2字节)
            uint16_t rgb16 = (r << 10) | (g << 5) | b;

            // 写入新的16位颜色数据
            fwrite(&rgb16, sizeof(uint16_t), 1, outputFile);
        }
        // 跳过原始图像的填充字节
        fseek(inputFile, padding_24bit, SEEK_CUR);

        uint16_t zero = 0;
        if (padding_16bit > 0)
        {
            fwrite(&zero, sizeof(uint16_t), 1, outputFile);
        }
        // fwrite(&zero, sizeof(uint8_t), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main() {
    const char *inputPath = "B:\\MyCode\\CLion_Code\\image_bmp_convert\\image_24bit.bmp";
    const char *outputPath = "B:\\MyCode\\CLion_Code\\image_bmp_convert\\image_16bit.bmp";

    // convert32To24Bit(inputPath, outputPath);
    // convert24To18Bit(inputPath, outputPath);
    convert24To16Bit(inputPath, outputPath);

    return 0;
}

