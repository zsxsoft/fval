Fval
=====================
[![Build Status](https://travis-ci.org/zsxsoft/fval.svg?branch=master)](https://travis-ci.org/zsxsoft/fval)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/zsxsoft/fval)](https://ci.appveyor.com/project/zsxsoft/fval/branch/master)

Fval(say F-word to eval) extension used to disable unsafe functions/eval with E_FATAL.

Based on [taint](https://github.com/laruence/taint). See: http://www.laruence.com/2012/02/18/2560.html (Chinese)

## Requires
PHP 7+

## Features
1. Disable ``eval``
1. Disable unsafe functions: ``passthru`` ``exec`` ``system`` ``shell_exec`` ``proc_open`` ``popen``
1. Throw error when ``include`` or ``require`` a file generate by PHP.

## Build
### *nix
```bash
phpize
./configure
make
make install
```

### Windows

If you're using PHP 7.1 + Non Thread Safe + x64, run ``ci\appveyor.bat`` with Administrator, it will auto compile to ``php-sdk\phpdev\vc14\x64\php-7.1.8\x64\Release\php-fval.dll``. Otherwise, follow https://github.com/OSTC/php-sdk-binary-tools/ and configure with ``--enable-fval=shared``.

## Configure
```ini
fval.disable_eval = 1
fval.disable_functions = 1
fval.enable = 1
fval.error_level = 512
```

## Questions

### Which of the functions used to write files will be recorded?

```php
fopen($fileName, "[awxc]");
file_put_contents($fileName);
copy(..., $dest);
move_uploaded_files(..., $dest);
```

### Which of the functions will be recorded soon?

```php
resource bzopen ( mixed $file , string $mode );
resource gzopen ( string $filename , string $mode [, int $use_include_path = 0 ] );
// do not record phar constructor
// phar.readonly is enabled by default for security reasons.
public bool Phar::copy ( string $oldfile , string $newfile );
public bool Phar::extractTo ( string $pathto [, string|array $files [, bool $overwrite = false ]] );
public bool RarEntry::extract ( string $dir [, string $filepath = "" [, string $password = NULL [, bool $extended_data = false ]]] )
bool ZipArchive::extractTo ( string $destination [, mixed $entries ] )
// Hide in cert
bool openssl_csr_export_to_file ( mixed $csr , string $outfilename [, bool $notext = true ] );
bool openssl_pkcs12_export_to_file ( mixed $x509 , string $filename , mixed $priv_key , string $pass [, array $args ] );
bool openssl_pkey_export_to_file ( mixed $key , string $outfilename [, string $passphrase [, array $configargs ]] );
bool openssl_x509_export_to_file ( mixed $x509 , string $outfilename [, bool $notext = TRUE ] );

resource dio_open ( string $filename , int $flags [, int $mode = 0 ] );

// Hide in EXIF
bool imagegd2 ( resource $image [, string $filename [, int $chunk_size [, int $type = IMG_GD2_RAW ]]] );
bool imagegd ( resource $image [, string $filename ] );
bool imagegif ( resource $image [, string $filename ] );
bool imagejpeg ( resource $image [, string $filename [, int $quality ]] );
bool imagepng ( resource $image [, string $filename ] );
bool imagewbmp ( resource $image [, string $filename [, int $foreground ]] );
bool imagewebp ( resource $image , mixed $to [, int $quality = 80 ] );
bool imagexbm ( resource $image , string $filename [, int $foreground ] );
public Gmagick Gmagick::setfilename ( string $filename );
public Gmagick Gmagick::setimagefilename ( string $filename );
Gmagick::write
public Gmagick Gmagick::writeimage ( string $filename [, bool $all_frames = false ] );

// Hide in SQLite

// TODO...
```

### Any known issues?
1. Not all the file will be recorded, for example, ``sqlite3_query('INSERT INTO')``. That means the hacker can eval PHP code hide in somewhere like image EXIF.

## TODO
1. [ ] Use hook or ``ptrace`` to record all file writing.
1. [ ] ``.phpt`` tests.
1. [ ] Dynamic calls detection.
1. [ ] PHP 5.6 Support
1. [ ] Support more PHP Functions.

## License

The PHP License