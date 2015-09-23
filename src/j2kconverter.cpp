#include <iostream>
#include <opj_config.h>
#include <openjpeg.h>
#include <j2kconverter.h>
#include <vector>
#include <cstring>

namespace {
    using namespace basicj2k;

    void errorCallback(const char *msg, void *clientData) {
        (void) clientData;
        std::cout << "ERROR: " << msg;
    }

    void warningCallback(const char *msg, void *clientData) {
        (void) clientData;
        std::cout << "WARNING: " << msg;
    }

    void infoCallback(const char *msg, void *clientData) {
        (void) clientData;
        (void) msg;
        //std::cout << "INFO: " << msg;
    }

    OPJ_SIZE_T readFunction(void *buffer, OPJ_SIZE_T size, void *userData) {
        J2kConverter::MemoryStream* ms = reinterpret_cast<J2kConverter::MemoryStream*>(userData);
        size = std::min(size, ms->data.size() - ms->offset);
        memcpy(buffer, ms->data.data() + ms->offset, size);
        return ms->offset += size;
    }
    
    OPJ_SIZE_T writeFunction(void *buffer, OPJ_SIZE_T size, void *userData) {
        J2kConverter::MemoryStream* ms = reinterpret_cast<J2kConverter::MemoryStream*>(userData);
        if (ms->offset + size > ms->data.size()) {
            ms->data.resize(ms->offset + size);
        }
        memcpy(ms->data.data() + ms->offset, buffer, size);
        return ms->offset += size;
    }

    OPJ_SIZE_T skipFunction(OPJ_OFF_T off, void *userData) {
        J2kConverter::MemoryStream* ms = reinterpret_cast<J2kConverter::MemoryStream*>(userData);
        ms->offset += off;
        return ms->offset;
    }

    OPJ_BOOL seekFunction(OPJ_OFF_T off, void *userData) {
        J2kConverter::MemoryStream* ms = reinterpret_cast<J2kConverter::MemoryStream*>(userData);
        ms->offset = off;
        return true;
    }

    void freeFunction(void *userData) {
        (void) userData;
        return;
    }

    opj_stream_t* createOpjMemoryStream(bool in, J2kConverter::MemoryStream* ms) {
        void* userData = reinterpret_cast<void*>(ms);
        opj_stream_t *stream = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, in);
        opj_stream_set_user_data(stream, userData, (opj_stream_free_user_data_fn) freeFunction);
        opj_stream_set_user_data_length(stream, ms->data.size());
        opj_stream_set_read_function(stream, (opj_stream_read_fn) readFunction);
        opj_stream_set_write_function(stream, (opj_stream_write_fn) writeFunction);
        opj_stream_set_skip_function(stream, (opj_stream_skip_fn) skipFunction);
        opj_stream_set_seek_function(stream, (opj_stream_seek_fn) seekFunction);
        return stream;
    }
}

namespace basicj2k {

char* J2kConverter::encode(int16_t* data, unsigned int imageWidth, unsigned int imageHeight, size_t& outSize, float distoRatio) {
    opj_cparameters_t params;
    opj_image_cmptparm_t compParams;

    opj_codec_t *codec;
    opj_image_t *image;
    opj_stream_t * stream;
    
    
    opj_set_default_encoder_parameters(&params);
    params.tcp_numlayers = 1;
    params.cp_fixed_quality = 1;
    params.tcp_distoratio[0] = distoRatio;
    
    params.cp_tx0 = 0;
    params.cp_ty0 = 0;
    params.tile_size_on = OPJ_FALSE;
    params.irreversible = true;

    params.numresolution = 1;
    params.prog_order = OPJ_LRCP;

    compParams.w = imageWidth;
    compParams.h = imageHeight;
    compParams.sgnd = 0;
    compParams.prec = 16;
    compParams.bpp = 16;
    compParams.x0 = 0;
    compParams.y0 = 0;
    compParams.dx = 1;
    compParams.dy = 1;
    
    codec = opj_create_compress(OPJ_CODEC_J2K);

    opj_set_info_handler(codec, infoCallback, nullptr);
    opj_set_warning_handler(codec, warningCallback, nullptr);
    opj_set_error_handler(codec, errorCallback, nullptr);

    image = opj_image_create(1, &compParams, OPJ_CLRSPC_GRAY);
    if (!image) {
        opj_destroy_codec(codec);
        return nullptr;
    }
    
    unsigned int nPixels = imageWidth * imageHeight;
    for (unsigned int i = 0; i < nPixels; ++i) {
        image->comps[0].data[i] = data[i];
    }
    
    image->x0 = 0;
    image->y0 = 0;
    image->x1 = static_cast<OPJ_UINT32>(imageWidth);
    image->y1 = static_cast<OPJ_UINT32>(imageHeight);
    image->color_space = OPJ_CLRSPC_GRAY;
    
    if (!opj_setup_encoder(codec, &params, image)) {
        std::cout << "Could not set up encoder." << std::endl;
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return nullptr;
    }

    MemoryStream* ms = new MemoryStream;
    stream = createOpjMemoryStream(OPJ_FALSE, ms);
    if (!stream) {
        std::cout << "Could not set up stream" << std::endl;
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return nullptr;
    }

    if (!opj_start_compress(codec, image, stream)) {
        std::cout << "Could not start compression." << std::endl;
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return nullptr;
    }

    if (!opj_encode(codec, stream)) {
        std::cout << "Could not encode data." << std::endl;
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return nullptr;
    }

    if (!opj_end_compress(codec, stream)) {
        std::cout << "Could not end compression." << std::endl;
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        return nullptr;
    }

    outSize = ms->data.size();
    char* outData = new char[outSize];
    memcpy(outData, ms->data.data(), outSize);
    
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    delete ms;
    return outData;
}

int16_t* J2kConverter::decode(char* inData, size_t inSize, unsigned int& imageWidth, unsigned int& imageHeight , int16_t* outData) {

    opj_image_t *image;
    opj_stream_t *stream;
    opj_codec_t *codec;
    opj_dparameters_t core;

    MemoryStream* ms = new MemoryStream;
    ms->data.resize(inSize);
    memcpy(ms->data.data(), inData, inSize);

    stream = createOpjMemoryStream(OPJ_TRUE, ms);
    if (!stream) {
        std::cout << "Could not create in stream." << std::endl;
        delete ms;
        return nullptr;
    }

    codec = opj_create_decompress(OPJ_CODEC_J2K);
    
    opj_set_info_handler(codec, infoCallback, nullptr);
    opj_set_warning_handler(codec, warningCallback, nullptr);
    opj_set_error_handler(codec, errorCallback, nullptr);

    if ( !opj_setup_decoder(codec, &core) ){
        std::cout << "Could not set up decoder." << std::endl;
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        delete ms;
        return nullptr;
    }

    if(! opj_read_header(stream, codec, &image)){
        std::cout << "Could not read header." << std::endl;
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        delete ms;
        return nullptr;
    }

    if (!(opj_decode(codec, stream, image))) {
        std::cout << "Could not decode image." << std::endl;
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        delete ms;
        return nullptr;
    }

    if (!opj_end_decompress(codec, stream)) {
        std::cout << "Could not end decompression." << std::endl;
        opj_destroy_codec(codec);
        opj_stream_destroy(stream);
        opj_image_destroy(image);
        delete ms;
        return nullptr;
    }


    imageWidth = image->x1 - image->x0;
    imageHeight = image->y1 - image->y0;
    unsigned int nPixels = imageWidth * imageHeight;
    
    if (outData == nullptr) {
        outData = new int16_t[nPixels];
    }

    for (unsigned int i = 0; i < nPixels; ++i) {
       outData[i] = image->comps[0].data[i];
    }
    
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    return outData; 
}
}
