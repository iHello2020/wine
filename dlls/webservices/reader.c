/*
 * Copyright 2015 Hans Leidekker for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "webservices.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(webservices);

static inline void *heap_alloc( SIZE_T size )
{
    return HeapAlloc( GetProcessHeap(), 0, size );
}

static inline void *heap_alloc_zero( SIZE_T size )
{
    return HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size );
}

static inline void *heap_realloc( void *mem, SIZE_T size )
{
    return HeapReAlloc( GetProcessHeap(), 0, mem, size );
}

static inline BOOL heap_free( void *mem )
{
    return HeapFree( GetProcessHeap(), 0, mem );
}

static const struct
{
    ULONG size;
    BOOL  readonly;
}
error_props[] =
{
    { sizeof(ULONG), TRUE },    /* WS_ERROR_PROPERTY_STRING_COUNT */
    { sizeof(ULONG), FALSE },   /* WS_ERROR_PROPERTY_ORIGINAL_ERROR_CODE */
    { sizeof(LANGID), FALSE }   /* WS_ERROR_PROPERTY_LANGID */
};

struct error
{
    ULONG             prop_count;
    WS_ERROR_PROPERTY prop[sizeof(error_props)/sizeof(error_props[0])];
};

static struct error *alloc_error(void)
{
    static const ULONG count = sizeof(error_props)/sizeof(error_props[0]);
    struct error *ret;
    ULONG i, size = sizeof(*ret) + count * sizeof(WS_ERROR_PROPERTY);
    char *ptr;

    for (i = 0; i < count; i++) size += error_props[i].size;
    if (!(ret = heap_alloc_zero( size ))) return NULL;

    ptr = (char *)&ret->prop[count];
    for (i = 0; i < count; i++)
    {
        ret->prop[i].value = ptr;
        ret->prop[i].valueSize = error_props[i].size;
        ptr += ret->prop[i].valueSize;
    }
    ret->prop_count = count;
    return ret;
}

static HRESULT set_error_prop( struct error *error, WS_ERROR_PROPERTY_ID id, const void *value, ULONG size )
{
    if (id >= error->prop_count || size != error_props[id].size || error_props[id].readonly)
        return E_INVALIDARG;

    memcpy( error->prop[id].value, value, size );
    return S_OK;
}

static HRESULT get_error_prop( struct error *error, WS_ERROR_PROPERTY_ID id, void *buf, ULONG size )
{
    if (id >= error->prop_count || size != error_props[id].size)
        return E_INVALIDARG;

    memcpy( buf, error->prop[id].value, error->prop[id].valueSize );
    return S_OK;
}

/**************************************************************************
 *          WsCreateError		[webservices.@]
 */
HRESULT WINAPI WsCreateError( const WS_ERROR_PROPERTY *properties, ULONG count, WS_ERROR **handle )
{
    struct error *error;
    LANGID langid = GetUserDefaultUILanguage();
    HRESULT hr;
    ULONG i;

    TRACE( "%p %u %p\n", properties, count, handle );

    if (!handle) return E_INVALIDARG;
    if (!(error = alloc_error())) return E_OUTOFMEMORY;

    set_error_prop( error, WS_ERROR_PROPERTY_LANGID, &langid, sizeof(langid) );
    for (i = 0; i < count; i++)
    {
        if (properties[i].id == WS_ERROR_PROPERTY_ORIGINAL_ERROR_CODE)
        {
            heap_free( error );
            return E_INVALIDARG;
        }
        hr = set_error_prop( error, properties[i].id, properties[i].value, properties[i].valueSize );
        if (hr != S_OK)
        {
            heap_free( error );
            return hr;
        }
    }

    *handle = (WS_ERROR *)error;
    return S_OK;
}

/**************************************************************************
 *          WsFreeError		[webservices.@]
 */
void WINAPI WsFreeError( WS_ERROR *handle )
{
    struct error *error = (struct error *)handle;

    TRACE( "%p\n", handle );
    heap_free( error );
}

static const struct
{
    ULONG size;
    BOOL  readonly;
}
heap_props[] =
{
    { sizeof(SIZE_T), FALSE }, /* WS_HEAP_PROPERTY_MAX_SIZE */
    { sizeof(SIZE_T), FALSE }, /* WS_HEAP_PROPERTY_TRIM_SIZE */
    { sizeof(SIZE_T), TRUE },  /* WS_HEAP_PROPERTY_REQUESTED_SIZE */
    { sizeof(SIZE_T), TRUE }   /* WS_HEAP_PROPERTY_ACTUAL_SIZE */
};

struct heap
{
    HANDLE           handle;
    ULONG            prop_count;
    WS_HEAP_PROPERTY prop[sizeof(heap_props)/sizeof(heap_props[0])];
};

static struct heap *alloc_heap(void)
{
    static const ULONG count = sizeof(heap_props)/sizeof(heap_props[0]);
    struct heap *ret;
    ULONG i, size = sizeof(*ret) + count * sizeof(WS_HEAP_PROPERTY);
    char *ptr;

    for (i = 0; i < count; i++) size += heap_props[i].size;
    if (!(ret = heap_alloc_zero( size ))) return NULL;

    ptr = (char *)&ret->prop[count];
    for (i = 0; i < count; i++)
    {
        ret->prop[i].value = ptr;
        ret->prop[i].valueSize = heap_props[i].size;
        ptr += ret->prop[i].valueSize;
    }
    ret->prop_count = count;
    return ret;
}

static HRESULT set_heap_prop( struct heap *heap, WS_HEAP_PROPERTY_ID id, const void *value, ULONG size )
{
    if (id >= heap->prop_count || size != heap_props[id].size || heap_props[id].readonly)
        return E_INVALIDARG;

    memcpy( heap->prop[id].value, value, size );
    return S_OK;
}

static HRESULT get_heap_prop( struct heap *heap, WS_HEAP_PROPERTY_ID id, void *buf, ULONG size )
{
    if (id >= heap->prop_count || size != heap_props[id].size)
        return E_INVALIDARG;

    memcpy( buf, heap->prop[id].value, heap->prop[id].valueSize );
    return S_OK;
}

/**************************************************************************
 *          WsCreateHeap		[webservices.@]
 */
HRESULT WINAPI WsCreateHeap( SIZE_T max_size, SIZE_T trim_size, const WS_HEAP_PROPERTY *properties,
                             ULONG count, WS_HEAP **handle, WS_ERROR *error )
{
    struct heap *heap;

    TRACE( "%u %u %p %u %p %p\n", (ULONG)max_size, (ULONG)trim_size, properties, count, handle, error );
    if (error) FIXME( "ignoring error parameter\n" );

    if (!handle || count) return E_INVALIDARG;
    if (!(heap = alloc_heap())) return E_OUTOFMEMORY;

    set_heap_prop( heap, WS_HEAP_PROPERTY_MAX_SIZE, &max_size, sizeof(max_size) );
    set_heap_prop( heap, WS_HEAP_PROPERTY_TRIM_SIZE, &trim_size, sizeof(trim_size) );

    if (!(heap->handle = HeapCreate( 0, 0, max_size )))
    {
        heap_free( heap );
        return E_OUTOFMEMORY;
    }

    *handle = (WS_HEAP *)heap;
    return S_OK;
}

/**************************************************************************
 *          WsFreeHeap		[webservices.@]
 */
void WINAPI WsFreeHeap( WS_HEAP *handle )
{
    struct heap *heap = (struct heap *)handle;

    TRACE( "%p\n", handle );

    if (!heap) return;
    HeapDestroy( heap->handle );
    heap_free( heap );
}

/**************************************************************************
 *          WsGetErrorProperty		[webservices.@]
 */
HRESULT WINAPI WsGetErrorProperty( WS_ERROR *handle, WS_ERROR_PROPERTY_ID id, void *buf,
                                   ULONG size )
{
    struct error *error = (struct error *)handle;

    TRACE( "%p %u %p %u\n", handle, id, buf, size );
    return get_error_prop( error, id, buf, size );
}

/**************************************************************************
 *          WsGetHeapProperty		[webservices.@]
 */
HRESULT WINAPI WsGetHeapProperty( WS_HEAP *handle, WS_HEAP_PROPERTY_ID id, void *buf,
                                  ULONG size, WS_ERROR *error )
{
    struct heap *heap = (struct heap *)handle;

    TRACE( "%p %u %p %u %p\n", handle, id, buf, size, error );
    if (error) FIXME( "ignoring error parameter\n" );

    return get_heap_prop( heap, id, buf, size );
}

/**************************************************************************
 *          WsSetErrorProperty		[webservices.@]
 */
HRESULT WINAPI WsSetErrorProperty( WS_ERROR *handle, WS_ERROR_PROPERTY_ID id, const void *value,
                                   ULONG size )
{
    struct error *error = (struct error *)handle;

    TRACE( "%p %u %p %u\n", handle, id, value, size );

    if (id == WS_ERROR_PROPERTY_LANGID) return WS_E_INVALID_OPERATION;
    return set_error_prop( error, id, value, size );
}
