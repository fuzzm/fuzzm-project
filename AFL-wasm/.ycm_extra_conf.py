import os

def Settings( **kwargs ):
  wasmtime_c_api = os.environ.get('WASMTIME_C_API')
  return {
    'flags': [ f'-I{wasmtime_c_api}/include', f'-L{wasmtime_c_api}/lib', '-lwasmtime' ],
  }
