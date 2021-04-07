export interface BenchmarkInfo {
  binName: string;
  cmd: string;
  outputFile: string;
  useRelativeInput?: boolean;
}

export const benchmarks = {
  openjpeg: {
    binName: 'bin/opj_compress.wasm',
    cmd: '-i @@ -o ##.jp2',
    outputFile: '##.jp2',
  },
  'tiff-4.0.9': {
    binName: 'tools/pal2rgb.wasm',
    cmd: '@@ ##',
    outputFile: '##',
  },
  libpng: {
    binName: 'contrib/pngminus/pnm2png.wasm',
    cmd: '@@ ##.png',
    outputFile: '##.png',
  },
  pdfresurrect: {
    binName: 'pdfresurrect.wasm',
    cmd: '@@',
    outputFile: 'stdout',
  },
  abc2mtex: {
    binName: 'abc2mtex.wasm',
    cmd: '@@',
    outputFile: 'stdout',
    useRelativeInput: true,
  },
  flac: {
    binName: 'src/flac/flac.wasm',
    cmd: '-f -d -o ##.wav @@',
    outputFile: '##.wav',
  },
  jbig2dec: {
    binName: 'jbig2dec.wasm',
    cmd: '-t png -o ##.dec @@',
    outputFile: '##.dec',
  },
};
