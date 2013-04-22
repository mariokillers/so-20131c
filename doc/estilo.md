# Guidelines para estilo de código

* Usar TABS para indentación, no espacios
* Siempre usar llaves para bloques, incluso si tienen una sola sentencia
* Espacio entre bloque condicional y condición, ej.: `if (...)`,
pero no en definición o invocación de funciones: `func();`
* Intentar limitarse a máximo 80 columnas (tomando como referencia TAB
de 4 espacios) de ancho a menos que sea absolutamente inevitable. Es muy
útil para comparar archivos lado a lado e imprimirlos si es necesario.
* Usar mensajes de commit descriptivos, consultar con los demás antes
de pushear al repo para evitar conflictos
* Evitar usar magic numbers, usar `#define`s y `enum`s cuando sea
posible
* Agregar un espacio antes y después de `+`, `-`, `=`, `==`, `!=`, etc.
* Usar comentarios cuando no es inmediatamente obvio qué hace un código.
Comentarios como éste serán penados con toda la severidad de la ley:

```
/* Si i es cero, saltar */
if (i == 0) {
    saltar();
}
```
* Para especificar un puntero, poner el asterisco delante del puntero,
no el tipo de dato. Ejemplo: `char *s`.
* Los [include guards](http://en.wikipedia.org/wiki/Include_guard) para
bibliotecas siguen la misma convención que la biblioteca commons.
* Si una variable no es modificada por una función, agregarle el prefijo
`const`.

## Bloques
```
if (condicion) {
    algo();
} else if (otracondicion) {
    otracosa();
} else {
    cosas();
}
```
```
for (i = 0; i < n; i++) {
    hola();
}
```
```
switch (condicion) {
    case 1:
        casouno();
        break;
    case 2:
	casodos();
        break;
}
```
```
struct larga {
    t_list lista;
    char *contenido;
    bool libre;
};
```
```
struct corta {int x; int y};
```

## Definición de funciones
```
int funcion(const char *s)
{
    while (*s++) {
        putc(*s);
    }
    return 0;
}
```    
```	
funcion_demasiado_larga_muchos_argumentos(int arg1, char *s,
                                          double otro_arg)
{
    hacer_magia();
    return EXIT_SUCCESSFUL;
}
```
