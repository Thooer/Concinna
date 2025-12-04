extern crate proc_macro;
use proc_macro::TokenStream;
use quote::quote;
use syn::{parse_macro_input, DeriveInput, Data, Fields, Meta, LitStr};

#[proc_macro_derive(Reflect)]
pub fn derive_reflect(input: TokenStream) -> TokenStream {
    let ast = parse_macro_input!(input as DeriveInput);
    let _ident = ast.ident;
    let gen = quote! { /* Reflect stub */ };
    gen.into()
}

#[proc_macro_derive(SoA, attributes(soa))]
pub fn derive_soa(input: TokenStream) -> TokenStream {
    let ast = parse_macro_input!(input as DeriveInput);
    let ident = ast.ident.clone();
    let soa_ident = syn::Ident::new(&format!("{}SoA", ident), ident.span());
    let mod_ident = syn::Ident::new(&format!("__soa_generated__{}", ident), ident.span());

    // Parse struct-level default mode
    let mut default_mode: Option<syn::Ident> = None;
    for attr in ast.attrs.iter() {
        if attr.path().is_ident("soa") {
            let mut mode_opt: Option<syn::Ident> = None;
            let _ = attr.parse_nested_meta(|meta| {
                if meta.path.is_ident("mode") {
                    let s: LitStr = meta.value()?.parse()?;
                    mode_opt = Some(syn::Ident::new(&s.value(), s.span()));
                    Ok(())
                } else { Ok(()) }
            });
            if mode_opt.is_some() { default_mode = mode_opt; }
        }
    }
    let default_mode_path = match default_mode {
        Some(m) => quote! { cap_containers::#m },
        None => quote! { cap_containers::UseVector },
    };

    // Collect fields, types, and per-field mode overrides
    let mut field_idents: Vec<syn::Ident> = Vec::new();
    let mut field_types: Vec<syn::Type> = Vec::new();
    let mut field_modes: Vec<Option<syn::Ident>> = Vec::new();

    if let Data::Struct(ds) = &ast.data {
        if let Fields::Named(named) = &ds.fields {
            for f in named.named.iter() {
                let fname = f.ident.clone().unwrap();
                let fty = f.ty.clone();
                // Parse field-level override
                let mut override_mode: Option<syn::Ident> = None;
                for a in f.attrs.iter() {
                    if a.path().is_ident("soa") {
                        let _ = a.parse_nested_meta(|meta| {
                            if meta.path.is_ident("mode") {
                                let s: LitStr = meta.value()?.parse()?;
                                override_mode = Some(syn::Ident::new(&s.value(), s.span()));
                                Ok(())
                            } else { Ok(()) }
                        });
                    }
                }
                field_idents.push(fname);
                field_types.push(fty);
                field_modes.push(override_mode);
            }
        }
    }

    // First field for len()
    let first_field_ident = field_idents.first().cloned();
    let first_field_type = field_types.first().cloned();

    // Build field storage types and where clauses
    let storage_tys: Vec<proc_macro2::TokenStream> = field_types
        .iter()
        .zip(field_modes.iter())
        .map(|(fty, mode)| {
            if let Some(m) = mode {
                quote! { <#fty as cap_containers::SoA<cap_containers::#m>>::Storage<'a> }
            } else {
                quote! { <#fty as cap_containers::SoA<M>>::Storage<'a> }
            }
        })
        .collect();

    let where_clauses: Vec<proc_macro2::TokenStream> = field_types
        .iter()
        .zip(field_modes.iter())
        .map(|(fty, mode)| {
            if let Some(m) = mode {
                quote! { #fty: cap_containers::SoA<cap_containers::#m> }
            } else {
                quote! { #fty: cap_containers::SoA<M> }
            }
        })
        .collect();

    // Build field declarations
    let field_decls: Vec<proc_macro2::TokenStream> = field_idents
        .iter()
        .zip(storage_tys.iter())
        .map(|(id, sty)| quote! { pub #id: #sty })
        .collect();

    // Build initialization for with_capacity / aligned
    let init_with_capacity: Vec<proc_macro2::TokenStream> = field_idents
        .iter()
        .zip(field_types.iter())
        .zip(field_modes.iter())
        .map(|((id, fty), mode)| {
            let storage_ty = if let Some(m) = mode {
                quote! { <#fty as cap_containers::SoA<cap_containers::#m>>::Storage<'a> }
            } else {
                quote! { <#fty as cap_containers::SoA<M>>::Storage<'a> }
            };
            quote! { #id: <#storage_ty as cap_containers::SoAColumn<'a, #fty>>::with_capacity(alloc, capacity)? }
        })
        .collect();

    let init_with_capacity_aligned: Vec<proc_macro2::TokenStream> = field_idents
        .iter()
        .zip(field_types.iter())
        .zip(field_modes.iter())
        .map(|((id, fty), mode)| {
            let storage_ty = if let Some(m) = mode {
                quote! { <#fty as cap_containers::SoA<cap_containers::#m>>::Storage<'a> }
            } else {
                quote! { <#fty as cap_containers::SoA<M>>::Storage<'a> }
            };
            quote! { #id: <#storage_ty as cap_containers::SoAColumn<'a, #fty>>::with_capacity_aligned(alloc, capacity, align)? }
        })
        .collect();

    let pushes: Vec<proc_macro2::TokenStream> = field_idents
        .iter()
        .zip(field_types.iter())
        .map(|(id, fty)| quote! { cap_containers::SoAColumn<'a, #fty>::push(&mut self.#id, value.#id)?; })
        .collect();

    let len_expr = if let (Some(fid), Some(fty)) = (first_field_ident.clone(), first_field_type.clone()) {
        quote! { cap_containers::SoAColumn<'a, #fty>::len(&self.#fid) }
    } else {
        quote! { 0usize }
    };

    let gen = quote! {
        mod #mod_ident {
            use super::*;
            pub struct #soa_ident<'a, M = #default_mode_path>
            where #( #where_clauses ),*
            { #( #field_decls, )* }

            impl<'a, M> #soa_ident<'a, M>
            where #( #where_clauses ),*
            {
                pub fn with_capacity(alloc: cap_memory::Allocator<'a>, capacity: usize) -> Result<Self, cap_memory::MemoryError> {
                    Ok(Self {
                        #( #init_with_capacity, )*
                    })
                }

                pub fn with_capacity_aligned(alloc: cap_memory::Allocator<'a>, capacity: usize, align: usize) -> Result<Self, cap_memory::MemoryError> {
                    Ok(Self {
                        #( #init_with_capacity_aligned, )*
                    })
                }

                pub fn push(&mut self, value: #ident) -> Result<(), cap_memory::MemoryError> {
                    #( #pushes )*
                    Ok(())
                }

                pub fn len(&self) -> usize {
                    #len_expr
                }
            }

            pub use #soa_ident as __Storage;
        }
        pub use #mod_ident::#soa_ident;
    };
    gen.into()
}
