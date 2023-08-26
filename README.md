# Rust

### Purpose

This project aimed to develop a noise generator plugin inspired by the subtle crackle and dust of vinyl. Unlike conventional vinyl noise plugins that rely on pre-recorded audio or live capture methods, I aimed to synthesize these sounds purely through DSP techniques.

This repo includes:

- [viatordsp JUCE User Module](https://github.com/landonviator/viatordsp)
- [Projucer Project](https://github.com/landonviator/viator-rust/blob/main/viator-rust/viator-rust.jucer) for Rust

### How to download a build

You can get a build of Rust either from my [Gumroad](https://viatordsp.gumroad.com/l/zyfng) post or by downloading a build artifact from the repo's GitHub actions run.
From the [main code page](https://github.com/landonviator/viator-rust) navigate to the _Actions_ tab, click the latest green workflow run, then grab the appropraite build from the _Articats_ section as either Mac AU, Mac VST3, or Windows VST3.

### Drive

This GIF depicts an oscilloscope displaying Rust's waveshaping process. Here, the input parameter is set to its maximum, while the "Age" knob is adjusted from its minimum to maximum setting.

![Rust Oscilloscope Reading](https://github.com/landonviator/viator-rust/blob/main/rust-reading.gif)

### Age

This GIF illustrates the frequency response of the "Age" parameter. Notably, the distortion processing focuses on mid-range content, wherein higher "Age" values lead to intense volume amplification.

![Rust Age Parameter Response](https://github.com/landonviator/viator-rust/blob/main/rust-age.gif)

## Contributing

Participation in this open-source project is not only appreciated but also actively encouraged. Both individuals with programming expertise and those without can make valuable contributions in diverse ways:

#### Testing

The simplest way to contribute is by using the plugin in any way you like! All software has its share of bugs and glitches. Increased usage of the plugin will expedite the discovery and resolution of bugs. Different DAWs and hosts may exhibit varied behaviors, leading to different types of bugs. Additionally, considering potential use in non-DAW software, such as Elgato WaveLink, introduces additional considerations.

#### Plugin Development

If you're interested in contributing to the plugin's development, that's fantastic! Start by exploring the issues page. If you encounter no existing bugs and wish to introduce new features, fine-tune the DSP, or propose any other enhancements, feel free to submit a pull request (PR), which will be reviewed promptly.

## Support

If you like my work and would like to support me creating more audio applications, check out my [Patreon](https://www.patreon.com/ViatorDSP) where you can donate and download all of my current plugins! I also have a [GumRoad](https://viatordsp.gumroad.com/?_gl=1*18tqfoy*_ga*MTg2MjcxNDgyNS4xNjg5OTI3NDE3*_ga_6LJN6D94N6*MTY5MjM5NjQ1Ni4xODguMS4xNjkyMzk2NTExLjAuMC4w) store with my most recent plugins.

### References

This endeavor involved the adaptation of DSP techniques originally authored by Andy Farnell in his book [Designing Sound](https://mitpress.mit.edu/9780262014410/designing-sound/). Farnell employed the Pure Data audio language to fashion various sounds like fire crackles, crickets, and car horns. In this context, I transformed the fire crackling patches into C++ code, and through substantial modifications, tailored them to discover a method for generating crackling sounds that harmonized effectively with the characteristics of vinyl audio.

## License

MIT License

Copyright (c) 2023 Landon Viator

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
