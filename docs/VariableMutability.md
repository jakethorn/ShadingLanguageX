# Variable Mutability

## tl;dr

ShadingLanguageX variables are immutable by default. Use the `mutable` modifier to make them mutable.

## Introduction

In most C-based or shading programming languages, variables are mutable by default. To create a variable that is not
mutable, you typically use the `const` modifier or something similar. For example:

```c++
void main()
{
    int x = 1;
    x = 2; // OK

    const int y = 1;
    y = 2; // Error
}
```

Whereas in some other languages, like Rust, variables are immutable by default.

```rust
fn main()
{
    let mut x = 1;
    x = 2; // OK
    
    let y = 1;
    y = 2; // Error
}
```

When creating variables, it is important to give as much information about that variable as possible. Most commonly this
is done using the name of the variable. For example, `int user_age = 33;` is a much better name than `int ua = 33;`.
Modifiers also give important information about the variable and so should be used whenever appropriate. For example, 
`const string favourite_lang = "slx";` tells the reader that the variable is not going to change over the course of the 
program.

## Const or Mutable?

Okay, so, which language is correct? My feeling is that Rust (and other programming languages with default immutability)
are correct, and I'll explain my position with an analogy.

### For Mutable

Let's say we are having a conversation, and I warn you that no one is going to break into your house and steal your stuff
tonight. "... Okay", you reply, slightly confused. The warning was kind of pointless because you didn't expect someone to break into your
house tonight, and the warning itself is about a non-dangerous situation. On the other hand, if I warned you that someone _was_ going to
break into your house and steal your stuff tonight, you would be much more receptive and grateful towards the warning. 
Perhaps even change your locks.

If it's not clear, `const` is the first warning, while `mutable` is the second. When you declare a variable as `const`, 
you are promising to not break in and change it later, but the warning is a bit shallow. There is no danger in __not__ 
changing a variable, so why the warning? `mutable` on the other hand is a visible heads up to the reader that this variable
is going to change, and you cannot assume its value at any point in the program.

It's also the case that around 75% of variables do not change after their initial definition (in general, for all programming languages), and if all variables used 
the correct modifiers (which they should), it's much less typing by having immutability by default. I am 100% sure if I analysed
the C++ code for mxslc++, the most common word would be `const`... by far. 

### For Const

ShadingLanguageX is a tiny shading language in a sea (or at least a lake) of other shading languages. Nobody is going to be excited about 
memorising new or irregular language choices, syntax or terminology for such a niche language. So, it makes sense that 
ShadingLanguageX should try to be as simple as possible but also to follow the same principles and conventions as more established
shading languages, even if they are a little archaic.

It's also the case that ShadingLanguageX (or any shading language for that matter) does not support CPU-based threading,
and so there is no memory safety or optimisation benefit from having immutable variables. It is simply a readability benefit to the developer.  

## Conclusion

I've decided to have variables be immutable by default in ShadingLanguageX. At the end of the day, I created ShadingLanguageX
because I am interested in programming language syntax and design (otherwise I would've just made a GLSL or OSL to MaterialX parser),
and I want to create a well-designed language. Most often this means following in the footsteps of previous languages 
because they were designed by extremely intelligent people, and sometimes it means doing my own thing because I think
it's better. I have tried to make error messages due to immutable variables as helpful as possible to point users in the 
right direction, and I hope no one gets too frustrated with my decision after forgetting to add `mutable` to their variable definition for the 100th time.