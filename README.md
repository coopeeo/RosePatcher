# Rosé Patcher (Name subject to change)
This is Rosé Patcher! The Aroma plugin that revives Nintendo TVii icon from the Wii U Menu and Home Menu, as well as the applet from inside!


## Credits
- [Cooper](https://github.com/coopeeo) ([@coopeeo](https://github.com/coopeeo)): Lead Plugin Developer - TVii Config, Acquire Independent Service Token (<-- hi cooper here to say this was absolute hell), and plugin settings.
- [bartholomew lachance](https://github.com/gf2p8affineqb) (also known as [bl](https://github.com/gf2p8affineqb)) ([@gf2p8affineqb](https://github.com/gf2p8affineqb)): Plugin Developer - TVii icon patches on Wii U Menu as well as Home Menu.
- [Swirlz](https://github.com/itzswirlz) ([@ItzSwirlz](https://github.com/itzswirls)): Plugin Developer - Helped a lot with issues (and cooper's dumb brain).
- All the other people in the Aroma Discord (because of cooper not understanding anything and being an idiot).
- [@darcellapy](https://github.com/darcellapy): For the original [Vino Config Patcher](https://github.com/darcellapy/vino-config-patcher-plugin)
- All the other people who worked on this project, you can see them at [projectrose.cafe](https://projectrose.cafe)
- [Pretendo](https://pretendo.network)/[Ash](https://github.com/ashquarky) ([@ashquarky](https://github.com/ashquarky)) for CI/Building with GitHub Actions. (Notice: we are not affiliated with Pretendo, nor do they endorse us. I also simply took the things that Ash did and replicated it here.)

## Help

### My console is crashing at the Wii U Menu! I also have a Japan console!
Well we were not able to detect your console automatically, so on your sd card, go to <b>sd card</b> > <b>wiiu</b> > <b>environments</b> > <b>aroma</b> > <b>plugins</b> > <b>config</b>, and then open the the file named <b>rosepatcher.json</b>. After you have opened the file, change the <i>force_jpn_console</i> value from <b>false</b> to <b>true</b>