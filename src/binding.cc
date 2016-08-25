#include <node.h>
#include <nan.h>
#include <string.h>
#include <algorithm>

#include "aacenc_lib.h"
#include "aacdecoder_lib.h"
#include "wav_file.h"
#include "wavwriter.h"
#include "mpegFileRead.h"

using namespace v8;
using namespace node;

namespace nodeaac {

	class EncoderWorker : public Nan::AsyncWorker {
	public:
		EncoderWorker(Nan::Callback *callback, const char *inputFileName, const char *outputFileName, const int bitRate, const int sampleRate, const int channels):
			Nan::AsyncWorker(callback), inputFileName(inputFileName), outputFileName(outputFileName), bitRate(bitRate), sampleRate(sampleRate), channels(channels)
			{ }
		~EncoderWorker() { }

		void Execute() {
			static void* inBuffer[] = { inputBuffer, ancillaryBuffer, &metaDataSetup };
			static INT inBufferIds[] = { IN_AUDIO_DATA, IN_ANCILLRY_DATA,IN_METADATA_SETUP };
			static INT inBufferSize[] = { sizeof(inputBuffer), sizeof(ancillaryBuffer),sizeof(metaDataSetup) };
			static INT inBufferElSize[] = { sizeof(INT_PCM), sizeof(UCHAR),sizeof(AACENC_MetaData) };
			static void* outBuffer[] = { outputBuffer };
			static INT outBufferIds[] = { OUT_BITSTREAM_DATA };
			static INT outBufferSize[] = { sizeof(outputBuffer) };
			static INT outBufferElSize[] = { sizeof(UCHAR) };

			char percents[200];
		    float percent, old_percent = -1.0;
		    float bread = 0, fileread;
		    int header_type = 0;
		    int bitrate = 0;
		    float length = 0;

			FILE *outf = NULL;
			inBufDesc.numBufs = sizeof(inBuffer)/sizeof(void*);
			inBufDesc.bufs = (void**)&inBuffer;
			inBufDesc.bufferIdentifiers = inBufferIds;

			inBufDesc.bufSizes = inBufferSize;
			inBufDesc.bufElSizes = inBufferElSize;

			outBufDesc.numBufs = sizeof(outBuffer)/sizeof(void*);
			outBufDesc.bufs = (void**)&outBuffer;
			outBufDesc.bufferIdentifiers = outBufferIds;
			outBufDesc.bufSizes = outBufferSize;
			outBufDesc.bufElSizes = outBufferElSize;

			inargs.numAncBytes = 0;
			inargs.numInSamples = 0;
			HANDLE_AACENCODER hAacEncoder = NULL; /* encoder handle */
				if ((ErrorStatus = aacEncOpen(&hAacEncoder, 0, 0)) != AACENC_OK ) {
				printf("Something went wrong\n");
			}
			int wavOpen = WAV_InputOpen(&pWav, inputFileName);
			if(wavOpen != 0) {
				printf("Couldn't open the file %s %i\n", inputFileName, wavOpen);
			}

			ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 5);
			ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATEMODE, 0);


			if(sampleRate != NULL) {
				ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_SAMPLERATE, sampleRate);
			} else {
				ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_SAMPLERATE, pWav->header.sampleRate);
			}

			if(channels != NULL) {
				ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELMODE, channels);
			} else {
				ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELMODE, pWav->header.numChannels);
			}

			ErrorStatus = aacEncEncode(hAacEncoder, NULL, NULL, NULL, NULL);
			ErrorStatus = aacEncInfo(hAacEncoder, &encInfo);
			outf = fopen(outputFileName, "wb+");

			do {
				inargs.numInSamples += WAV_InputRead (pWav, &inputBuffer[inargs.numInSamples],FDKmin(encInfo.inputChannels*encInfo.frameLength,sizeof(inputBuffer) /sizeof(INT_PCM)-inargs.numInSamples),SAMPLE_BITS);

				bread+=(inargs.numInSamples*2);
				if(inargs.numInSamples == 0) {
					goto ENC_END;
				}

				ErrorStatus = aacEncEncode(hAacEncoder,
								&inBufDesc,
								&outBufDesc,
								&inargs,
								&outargs);

				if (outargs.numInSamples>0) {
					FDKmemmove(inputBuffer, &inputBuffer[outargs.numInSamples], sizeof(INT_PCM)*(inargs.numInSamples-outargs.numInSamples));
					inargs.numInSamples -= outargs.numInSamples;
				}

				if (outargs.numOutBytes>0) {
					fwrite(outputBuffer,1,outargs.numOutBytes,outf);
				}
			} while (ErrorStatus == AACENC_OK);

			ENC_END:
			fclose(outf);
			WAV_InputClose(&pWav);
		}

		void HandleOKCallback () {
			Nan::HandleScope scope;
			callback->Call(0, NULL);
		}
	private:
		const char *inputFileName;
		const char *outputFileName;
		const int bitRate;
		const int sampleRate;
		const int channels;
		int frame_cnt;

		AACENC_ERROR ErrorStatus;
		AACENC_InfoStruct encInfo;

		AACENC_BufDesc inBufDesc;
		AACENC_BufDesc outBufDesc;
		AACENC_InArgs inargs;
		AACENC_OutArgs outargs;
		HANDLE_WAV pWav;
		INT_PCM inputBuffer[8*2048];
		UCHAR ancillaryBuffer[50];
		AACENC_MetaData metaDataSetup;
		UCHAR outputBuffer[8192];
		UCHAR conf;
	};

	class DecoderWorker : public Nan::AsyncWorker {
	public:
		DecoderWorker(Nan::Callback *callback, const char *inputFileName, const char *outputFileName):
			Nan::AsyncWorker(callback), inputFileName(inputFileName), outputFileName(outputFileName)
			{ }
		~DecoderWorker() { }

		void Execute() {
			outputSize = 8*2*1024;
			outputBuffer = (uint8_t*) malloc(outputSize);

			decodeBuffer = (int16_t*) malloc(outputSize);
			aacDecoderInfo = aacDecoder_Open(TT_MP4_ADTS, 1);
			inputFile = fopen(inputFileName, "rb");
			while(1) {
				uint8_t packet[10240], *ptr = packet;
				int n, i;

				n = fread(packet, 1, 7, inputFile);

				if (n != 7) {
					break;
				}

				if (packet[0] != 0xff || (packet[1] & 0xf0) != 0xf0) {
					fprintf(stderr, "Not an ADTS packet\n");
					break;
				}

				const UINT packet_size = ((packet[3] & 0x03) << 11) | (packet[4] << 3) | (packet[5] >> 5);
				n = fread(packet + 7, 1, packet_size - 7, inputFile);

				if (n != packet_size - 7) {
					fprintf(stderr, "Partial packet\n");
					break;
				}

				valid = packet_size;
				errorStatus = aacDecoder_Fill(aacDecoderInfo, &ptr, &packet_size, &valid);
				if (errorStatus != AAC_DEC_OK) {
					fprintf(stderr, "Fill failed: %x\n", errorStatus);
					break;
				}
				errorStatus = aacDecoder_DecodeFrame(aacDecoderInfo, decodeBuffer, outputSize, 0);
				if (errorStatus == AAC_DEC_NOT_ENOUGH_BITS) {
					continue;
				}

				if (errorStatus != AAC_DEC_OK) {
					fprintf(stderr, "Decode failed: %x\n", errorStatus);
					continue;
				}

				if (!wav) {
					CStreamInfo *info = aacDecoder_GetStreamInfo(aacDecoderInfo);
					if (!info || info->sampleRate <= 0) {
						fprintf(stderr, "No stream info\n");
						break;
					}
					frame_size = info->frameSize * info->numChannels;
					// Note, this probably doesn't return channels > 2 in the right order for wav
					wav = wav_write_open(outputFileName, info->sampleRate, 16, info->numChannels);
					if (!wav) {
						perror(outputFileName);
						break;
					}
				}
				for (i = 0; i < frame_size; i++) {
					uint8_t* out = &outputBuffer[2*i];
					out[0] = decodeBuffer[i] & 0xff;
					out[1] = decodeBuffer[i] >> 8;
				}
				wav_write_data(wav, outputBuffer, 2*frame_size);
			}
			free(outputBuffer);
			free(decodeBuffer);
			fclose(inputFile);
			if (wav) {
				wav_write_close(wav);
			}

			aacDecoder_Close(aacDecoderInfo);
		}

		void HandleOKCallback () {
			Nan::HandleScope scope;
			callback->Call(0, NULL);
		}
	private:
		const char *inputFileName;
		const char *outputFileName;
		void *wav = NULL;
		int outputSize;
		FILE *inputFile;
		uint8_t *outputBuffer;
		int16_t *decodeBuffer;
		HANDLE_AACDECODER aacDecoderInfo;
		AAC_DECODER_ERROR errorStatus;
		UINT valid;
		int frame_size = 0;
	};

	static inline char *TO_CHAR(Handle<Value> val) {
	    String::Utf8Value utf8(val->ToString());

	    int len = utf8.length() + 1;
	    char *str = (char *) calloc(sizeof(char), len);
	    strncpy(str, *utf8, len);

	    return str;
	}

	NAN_METHOD(node_aac_encode) {
		Nan::HandleScope scope;

		Nan::MaybeLocal<String> n_inputFileName = Nan::To<String>(info[0]);
		v8::Local<String> inputFileName;
		n_inputFileName.ToLocal(&inputFileName);

		Nan::MaybeLocal<String> n_outputFileName = Nan::To<String>(info[1]);
		v8::Local<String> outputFileName;
		n_outputFileName.ToLocal(&outputFileName);

		int bitRate;
		int sampleRate;
		int channels;

		Nan::Maybe<int> n_bitRate = Nan::To<int>(info[2]);

		if(n_bitRate.IsNothing()) {
			bitRate = 112000;
		} else {
			bitRate = n_bitRate.FromJust();
		}

		Nan::Maybe<int> n_sampleRate = Nan::To<int>(info[3]);
		if(n_sampleRate.IsNothing()) {
			sampleRate = NULL;
		} else {
			sampleRate = n_sampleRate.FromJust();
		}

		Nan::Maybe<int> n_channels = Nan::To<int>(info[4]);
		if(n_channels.IsNothing()) {
			channels = NULL;
		} else {
			channels = n_channels.FromJust();
		}

		Nan::Callback *nanCallback = new Nan::Callback(info[5].As<Function>());
  		Nan::AsyncQueueWorker(new EncoderWorker(nanCallback, TO_CHAR(inputFileName), TO_CHAR(outputFileName), bitRate, sampleRate, channels));
	}

	NAN_METHOD(node_aac_decode) {
		Nan::HandleScope scope;

		Nan::MaybeLocal<String> n_inputFileName = Nan::To<String>(info[0]);
		v8::Local<String> inputFileName;
		n_inputFileName.ToLocal(&inputFileName);

		Nan::MaybeLocal<String> n_outputFileName = Nan::To<String>(info[1]);
		v8::Local<String> outputFileName;
		n_outputFileName.ToLocal(&outputFileName);

		Nan::Callback *nanCallback = new Nan::Callback(info[2].As<Function>());
  		Nan::AsyncQueueWorker(new DecoderWorker(nanCallback, TO_CHAR(inputFileName), TO_CHAR(outputFileName)));
	}

	NAN_MODULE_INIT(Initialize) {
		Nan::HandleScope scope;
		Nan::SetMethod(target, "encode", node_aac_encode);
		Nan::SetMethod(target, "decode", node_aac_decode);
	}
}

NODE_MODULE(aac, nodeaac::Initialize)
