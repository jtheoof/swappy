# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [1.5.1](https://github.com/jtheoof/swappy/compare/v1.5.0...v1.5.1) (2022-11-20)


### Bug Fixes

* **ui:** use *-symbolic variant of toolbar icons ([5dc44f8](https://github.com/jtheoof/swappy/commit/5dc44f8970b0f6cdf21466bc2689ec2aa93a4385)), closes [#34](https://github.com/jtheoof/swappy/issues/34)

## [1.5.0](https://github.com/jtheoof/swappy/compare/v1.4.0...v1.5.0) (2022-11-18)


### Features

* **config:** add early_exit option ([60da549](https://github.com/jtheoof/swappy/commit/60da5491e243c9edd85f6225326a68ae5e3edfd5))
* **config:** allow paint_mode to be configured through config file ([2f35f02](https://github.com/jtheoof/swappy/commit/2f35f02b4e89bf67b6e9cc461e874331d8ce2a4c))
* **config:** try to create `save_dir` if it does not exist ([4fb291a](https://github.com/jtheoof/swappy/commit/4fb291ad4b0b116afeaa7094b040083111b74674))
* **ui:** allow filling rectangles and ellipsis ([8ee55f7](https://github.com/jtheoof/swappy/commit/8ee55f7d52ce6ac71752981863f5795fef460049)), closes [#120](https://github.com/jtheoof/swappy/issues/120)

## [1.4.0](https://github.com/jtheoof/swappy/compare/v1.3.1...v1.4.0) (2021-09-06)


### Features

* **draw:** draw shape from center if holding control ([d80c361](https://github.com/jtheoof/swappy/commit/d80c3614895d3b5da479831c651cc1afa2fcf916))
* **i18n:** add french translations ([cacb283](https://github.com/jtheoof/swappy/commit/cacb2830e4cc41010d6ab96655054d2eb1651651))


### Bug Fixes

* **desktop:** remove annotation from desktop categories ([0d383f6](https://github.com/jtheoof/swappy/commit/0d383f690b99026c340eab1efa590c48d54e7368))
* **desktop:** various fixes ([42425c0](https://github.com/jtheoof/swappy/commit/42425c0657a65b3f66ba4f64b1727c8198a70684))
* **i18n:** add german translations to desktop file ([c6b09e5](https://github.com/jtheoof/swappy/commit/c6b09e56399369b14a8de090a2239350dbe4aca8))
* **i18n:** add turkish translation to desktop file ([fa5769e](https://github.com/jtheoof/swappy/commit/fa5769e9406b8ab1b67aca3bff2656850362491e))
* **i18n:** properly set translation domain during layout init ([5301aeb](https://github.com/jtheoof/swappy/commit/5301aebd5e5534453621db7168b8afac5d7810f2)), closes [#92](https://github.com/jtheoof/swappy/issues/92)
* **pixbuf:** handle invalid input file ([cdbd06d](https://github.com/jtheoof/swappy/commit/cdbd06d7af94b4aedfc2bda2231da8853f775f3a))
* **pixbuf:** handle overflow when filename_format is too long ([185575b](https://github.com/jtheoof/swappy/commit/185575bf75281eba8a0bc49b3da59225bdd9e1c7)), closes [#74](https://github.com/jtheoof/swappy/issues/74)
* **po:** update GETTEXT_PACKAGE value with project name ([7fd552e](https://github.com/jtheoof/swappy/commit/7fd552e8c41f29711212d7f70edf61ac6ada7a7d))
* **release:** properly check sha256 remote content ([91985c7](https://github.com/jtheoof/swappy/commit/91985c7994764f52c8e9d864db8ec9cf2eb1df5c)), closes [#90](https://github.com/jtheoof/swappy/issues/90)

### [1.3.1](https://github.com/jtheoof/swappy/compare/v1.3.0...v1.3.1) (2021-02-20)

## [1.3.0](https://github.com/jtheoof/swappy/compare/v1.2.1...v1.3.0) (2021-02-18)


### Features

* **cli:** add configure options for filename save ([597f005](https://github.com/jtheoof/swappy/commit/597f0055b9c6230b25a7f7a7bf3f4e14c06b1fbb))
* **i18n:** add brazilian portuguese translations ([4a0eb82](https://github.com/jtheoof/swappy/commit/4a0eb82369a0859fafdcce9d242c086cd2360a84))
* **i18n:** add german translations ([b4be847](https://github.com/jtheoof/swappy/commit/b4be8476350771454b29b9ce29c62a3337acc736))
* **i18n:** add turkish translations ([c8419da](https://github.com/jtheoof/swappy/commit/c8419da7faef14223ada6853942a6d11e2acf92f))


### Bug Fixes

* **application:** unlink temp file coming from stdin ([c24e56a](https://github.com/jtheoof/swappy/commit/c24e56a165394e60b37534287e168e5d8e69627c)), closes [#80](https://github.com/jtheoof/swappy/issues/80)
* **blur:** optimize blur to only render after commit ([27fcece](https://github.com/jtheoof/swappy/commit/27fcecedaeea49aaec6acdecbc51cbd865a13363))
* **blur:** rgb24 is properly handled ([c04ed63](https://github.com/jtheoof/swappy/commit/c04ed63d26e5012215198f7b41a7f2232dac1ebe))
* **clipboard:** wl-copy mimetype should be png ([a931acb](https://github.com/jtheoof/swappy/commit/a931acb2cff615badc63294ed121aba008f32ef8)), closes [#68](https://github.com/jtheoof/swappy/issues/68)
* **notification:** notification shows the image icon ([eb53e5c](https://github.com/jtheoof/swappy/commit/eb53e5c2b28717f509dd58eab6da85897c0d6d9d))
* **ui:** adjust rendering surface with proper scaling ([9b72571](https://github.com/jtheoof/swappy/commit/9b72571596f9313d4efd94a4b17da8b3733fd2de)), closes [#54](https://github.com/jtheoof/swappy/issues/54)
* **ui:** commit state before copying or saving ([46e5854](https://github.com/jtheoof/swappy/commit/46e5854b3cce93a82984b19ca90e3f3337952fe2)), closes [#52](https://github.com/jtheoof/swappy/issues/52)
* **ui:** compute window sizes and buffers properly ([5bcffdb](https://github.com/jtheoof/swappy/commit/5bcffdbb01cc6e56f9c0f37de899b46efe68ed4a)), closes [#56](https://github.com/jtheoof/swappy/issues/56)

### [1.2.1](https://github.com/jtheoof/swappy/compare/v1.2.0...v1.2.1) (2020-07-11)


### Bug Fixes

* **text:** properly handle utf-8 chars ([717ab0c](https://github.com/jtheoof/swappy/commit/717ab0c2d1757e10bb4eef17d35ccd6a991705c4)), closes [#43](https://github.com/jtheoof/swappy/issues/43)

## [1.2.0](https://github.com/jtheoof/swappy/compare/v1.1.0...v1.2.0) (2020-07-05)


### Features

* **i18n:** add translatable desktop file ([cf3d7a5](https://github.com/jtheoof/swappy/commit/cf3d7a5283a7b8c34b05996f87b608513e0830ca)), closes [#35](https://github.com/jtheoof/swappy/issues/35)
* **i18n:** setup i18n for swappy ([5b3c8ad](https://github.com/jtheoof/swappy/commit/5b3c8aded8fd4f9d00aa660a24127de0e1791d7f))

## [1.2.0](https://github.com/jtheoof/swappy/compare/v1.1.0...v1.2.0) (2020-07-05)


### Features

* **i18n:** add translatable desktop file ([cf3d7a5](https://github.com/jtheoof/swappy/commit/cf3d7a5283a7b8c34b05996f87b608513e0830ca)), closes [#35](https://github.com/jtheoof/swappy/issues/35)
* **i18n:** setup i18n for swappy ([5b3c8ad](https://github.com/jtheoof/swappy/commit/5b3c8aded8fd4f9d00aa660a24127de0e1791d7f))

## [1.1.0](https://github.com/jtheoof/swappy/compare/v1.0.1...v1.1.0) (2020-06-23)


### Features

* **cli:** add -v and --version flags ([e32c024](https://github.com/jtheoof/swappy/commit/e32c02454ae4ec6ac30549d5fa9e80c2b64edb72))

### [1.0.1](https://github.com/jtheoof/swappy/compare/v1.0.0...v1.0.1) (2020-06-21)


### Bug Fixes

* **cli:** stop showing -g option ([ee06d66](https://github.com/jtheoof/swappy/commit/ee06d6685f6f59ffce544b45d7b51f3f4523348b))

## 1.0.0 (2020-06-21)


### ⚠ BREAKING CHANGES

* We do no support the `-g` option anymore.

This tool simply makes more sense as the output of `grim` rather than
trying to be `grim`.

RIP my ugly wayland code, long live maintainable code.

Next stop, rust?

### Features

* **ui:** life is full of colors and joy ([a8c8be3](https://github.com/jtheoof/swappy/commit/a8c8be37ca996f3e1b752bca67eee594706bc08f))
* init project ([efc3ecc](https://github.com/jtheoof/swappy/commit/efc3eccc9e21892a6b0979126a23d21d3d6a3b3d))
* **application:** print final surface to file or stdout ([196f7f4](https://github.com/jtheoof/swappy/commit/196f7f4dea3ab569f0523171ae7c424b8e8423ee)), closes [#2](https://github.com/jtheoof/swappy/issues/2)
* **application:** update app ([ce27741](https://github.com/jtheoof/swappy/commit/ce27741017554d6606e23434273f55476bc8ae37))
* **blur:** add multiple passes logic ([f9737d7](https://github.com/jtheoof/swappy/commit/f9737d78c96a5d9f4566c94702c3ec4a41d9e219))
* **blur:** remove blur configuration ([361be6a](https://github.com/jtheoof/swappy/commit/361be6aa8085143d9fd721e4c315c6b9e6fbdfca))
* **blur:** use rect blur instead of brush ([1be7798](https://github.com/jtheoof/swappy/commit/1be7798a8bcfc494b20489e2e1f8b0245f4b5e84)), closes [#17](https://github.com/jtheoof/swappy/issues/17)
* **buffer:** ability to read from stdin ([02bc464](https://github.com/jtheoof/swappy/commit/02bc46456453e8530a3c9f1289dfce7e71371945))
* **buffer:** add file image support ([f6c189c](https://github.com/jtheoof/swappy/commit/f6c189c7b7f35ca4da75abaac0bd85c3d5ce5b09))
* **clipboard:** use wl-copy if present ([51b27d7](https://github.com/jtheoof/swappy/commit/51b27d768eef7fbbdab365fa94a81af5395b0e3e))
* **config:** add show_panel config ([307f579](https://github.com/jtheoof/swappy/commit/307f57956f105d22de2d8242313517b6a79ed4e2)), closes [#12](https://github.com/jtheoof/swappy/issues/12)
* **config:** have overridable defaults ([ef24851](https://github.com/jtheoof/swappy/commit/ef24851deec2d6b7f76ed0fbbcd31b54b336cae3)), closes [#1](https://github.com/jtheoof/swappy/issues/1)
* **draw:** convert wl_shm_format to cairo_format ([c623939](https://github.com/jtheoof/swappy/commit/c623939e02238f053312ad6367e761aec254c6fe))
* **draw:** draw the screencopy buffer ([2344414](https://github.com/jtheoof/swappy/commit/2344414102789975e6ce425a95e8b96159cf51ba))
* **layer:** use geometry size ([290d3ca](https://github.com/jtheoof/swappy/commit/290d3ca230d32ec2ef4036bf9e32f1e711fecd84))
* **paint:** introduce text paint ([3347bf2](https://github.com/jtheoof/swappy/commit/3347bf23bf17d4c2cc8e5b9bbadd657efafb28e7))
* **screencopy:** add buffer creation through screencopy ([bff8687](https://github.com/jtheoof/swappy/commit/bff8687fc81ebb57a179b1f50300f9c0cda793e3))
* **screencopy:** introduce screencopy features ([53c9770](https://github.com/jtheoof/swappy/commit/53c977080829c7e816db1a9ec45eb432f6b7b354))
* **swappy:** copy to clipboard with CTRL+C ([b90500e](https://github.com/jtheoof/swappy/commit/b90500ed34defcb8ebc67965c4dbb5d068ee8049))
* **swappy:** introduce file option ([c56df33](https://github.com/jtheoof/swappy/commit/c56df33d1880d22372e21ef0ebf5dd8805d65a76))
* **swappy:** save to file with CTRL+S ([af0b1a1](https://github.com/jtheoof/swappy/commit/af0b1a11a21faac04f8b43c4c9ef616ab5fd2b78))
* **text:** add controls in toggle panel ([c03f628](https://github.com/jtheoof/swappy/commit/c03f628de793e170d9f62c5b786fe18891bb6fa3))
* **tool:** introduce blurring capability ([fae0aea](https://github.com/jtheoof/swappy/commit/fae0aeacab6fb28e17975097c8b4c5c7e5ad57fd)), closes [#17](https://github.com/jtheoof/swappy/issues/17)
* **ui:** add binding for clear action ([2bdab68](https://github.com/jtheoof/swappy/commit/2bdab684e1eace53ad7b78414ad467d312dc10ad))
* **ui:** add binding to toggle panel ([e8d2f12](https://github.com/jtheoof/swappy/commit/e8d2f12ce1737fa19972e5c4109e1c85cc2b157e))
* **ui:** add keybindings for color change ([c5ec285](https://github.com/jtheoof/swappy/commit/c5ec285ee73ddf90df2cb571e1d6c61159605c8e))
* **ui:** add keybindings for stroke size ([562a9a6](https://github.com/jtheoof/swappy/commit/562a9a6e92201677f31de126b646c619caf33863))
* **ui:** add shortcuts for undo/redo ([d7e7f2b](https://github.com/jtheoof/swappy/commit/d7e7f2b5ffd46aa36bed6ecc6709aeb94cce64ae))
* **ui:** add toggle panel button ([7674d7d](https://github.com/jtheoof/swappy/commit/7674d7db8ba8d97302a045af8d2383de37acb2d1)), closes [#24](https://github.com/jtheoof/swappy/issues/24)
* **ui:** add undo/redo ([bcc1314](https://github.com/jtheoof/swappy/commit/bcc13140ebfdefa30431b288f089d23bb1df743e))
* **ui:** life is full of colors and joy ([606cd34](https://github.com/jtheoof/swappy/commit/606cd3459de3908e5fecdb7a49162ef3a9b52ab7))
* **ui:** replace popover by on screen elements ([8cd3f13](https://github.com/jtheoof/swappy/commit/8cd3f134bbd8e05523303914f6c8f3989e6b4502))
* **wayland:** added xdg_output_manager ([7b3549f](https://github.com/jtheoof/swappy/commit/7b3549fdd86fe1a945e1988bf22042c0f8dd6ed0))
* **wayland:** listing outputs ([5a55c8b](https://github.com/jtheoof/swappy/commit/5a55c8bbbd08ad717ddabac51be31483950d827f))


### Bug Fixes

* **application:** fix file loop and use of GTK object after lifecycle ([320dae0](https://github.com/jtheoof/swappy/commit/320dae02d0c6dca3fa2fd7ca934a85483ac2dd35))
* **application:** memory leak for pixbuf ([f9d70fc](https://github.com/jtheoof/swappy/commit/f9d70fc0e22274e6cbe74bfdf714cdf04e34053d))
* **application:** properly save output file upon clean exit ([b5cc433](https://github.com/jtheoof/swappy/commit/b5cc433d75d77759cef139e0e232bde79196f886)), closes [#8](https://github.com/jtheoof/swappy/issues/8)
* **application:** suffix saved file with png ([7f2f6da](https://github.com/jtheoof/swappy/commit/7f2f6da754571771475558233f5a47813ec278dd))
* **blur:** adjust blur bounding box based on scaled monitor ([6b2ec90](https://github.com/jtheoof/swappy/commit/6b2ec90efd99e1979310b673ad40b3724669dac1))
* **blur:** blur based on device scaling factor ([1699474](https://github.com/jtheoof/swappy/commit/1699474c39fc305492c8bb03063c4582af4dbf9e))
* **blur:** use better glyph icon ([97cd607](https://github.com/jtheoof/swappy/commit/97cd6072c986c9a7c69306744390a6ddb6a44646))
* **blur:** use rendered surface after commit ([46fb08d](https://github.com/jtheoof/swappy/commit/46fb08dce17a820fcb500d2b6ff02f7d682f3c18)), closes [#20](https://github.com/jtheoof/swappy/issues/20) [#22](https://github.com/jtheoof/swappy/issues/22)
* **buffer:** properly include required functions ([d787586](https://github.com/jtheoof/swappy/commit/d787586b9ed1d7e855ae2d416914d619636f41b1)), closes [#10](https://github.com/jtheoof/swappy/issues/10)
* **clipboard:** handle bad write to pipe fd ([f963a76](https://github.com/jtheoof/swappy/commit/f963a76c5c01b9b5f81b97118bf1b9e6990d995d))
* **clipboard:** memory leak for pixbuf ([665295b](https://github.com/jtheoof/swappy/commit/665295b497d7ef124d5a2eeb7eb76964fdb3566a))
* **dependencies:** include glib2 ([992d97e](https://github.com/jtheoof/swappy/commit/992d97e94d2ebd32ac3e1901910050fae1954ed0)), closes [#11](https://github.com/jtheoof/swappy/issues/11)
* **file:** properly check file system errors if any ([541ec21](https://github.com/jtheoof/swappy/commit/541ec21ca0efdec4d06c96f5ad1768b4219ed4ab))
* **init:** fix segfault for unknown flags ([f4e9a19](https://github.com/jtheoof/swappy/commit/f4e9a19407d8d1bfa59c08f6bf97617c662e1ac0))
* **init:** properly handle null geometry ([c4ea305](https://github.com/jtheoof/swappy/commit/c4ea305ae6ac9429bf44fdfc7218a30363439582))
* **man:** remove blur_level related config ([ceb907a](https://github.com/jtheoof/swappy/commit/ceb907a5dc736c7d44318b35fb911aeb2360d851))
* **meson:** able to build on standard platforms ([8abc5d5](https://github.com/jtheoof/swappy/commit/8abc5d52ec2962a111c6d44cdb5e9e209ac219c7))
* **meson:** remove useless cname in meson res file ([9b8ea64](https://github.com/jtheoof/swappy/commit/9b8ea64307b33eb010b8ba043919f3eddf935b19))
* **paint:** fix memory leak for brush paints ([aed2bfe](https://github.com/jtheoof/swappy/commit/aed2bfe29465aa5161155c1edda9d03cac607906))
* **pixbuf:** possibly fix core dump ([8a82e79](https://github.com/jtheoof/swappy/commit/8a82e796bb871b57fa6ab4d2ed8d761033370d8c))
* **pixbuf:** properly grab pixbuf size from cairo surface ([2adcf94](https://github.com/jtheoof/swappy/commit/2adcf944f4a7f2da5b5edf49a37922c43b2e477e)), closes [#6](https://github.com/jtheoof/swappy/issues/6)
* **render:** better handler empty buffer ([acf2379](https://github.com/jtheoof/swappy/commit/acf2379ba3117ba6eb8c426e85a60ce71a3abe67))
* **render:** draw from last to first ([4b69ada](https://github.com/jtheoof/swappy/commit/4b69ada9a1469d3b6e106e07bf7155836b31d613))
* **render:** fix arrow glitch with 0 ftx ([ec6e6ab](https://github.com/jtheoof/swappy/commit/ec6e6abae7629800fec4c715957c4932946f51ed))
* **render:** properly scale arrow along with stroke size ([75bfc10](https://github.com/jtheoof/swappy/commit/75bfc10fb7a5507b66bd6d19ab06f2f6a393bb6a))
* **resources:** compile resources and fix error management ([05d87c9](https://github.com/jtheoof/swappy/commit/05d87c929ff8b3311cd5db111cd2f53a32c35a19))
* **string:** fix algo to insert chars at location ([bc3264e](https://github.com/jtheoof/swappy/commit/bc3264e9f11bb4f3a02d7f5ae92ef8a4d2b42513))
* **ui:** add stroke size increase/decrease/reset ([5930c99](https://github.com/jtheoof/swappy/commit/5930c99b9e0208148d6bc8cf0fc3aa8f69dbd36d))
* **ui:** move paint area inside GtkFixed ([50e7c97](https://github.com/jtheoof/swappy/commit/50e7c97042805f5550d2a62d45c8e49208d7632d))
* **ui:** prevent focus in panel buttons ([903ad11](https://github.com/jtheoof/swappy/commit/903ad114f516981c8d0644f704af9c722f74a61f))
* **ui:** small tweaks ([2b73777](https://github.com/jtheoof/swappy/commit/2b73777142141598c14d37d1b6fa9573de12d914))
* **ui:** tweak button sizes ([425f455](https://github.com/jtheoof/swappy/commit/425f455ab7665a046060fe140c861aeb7ea8209b))
* **ui/render:** adjust rendering based on window size ([445980b](https://github.com/jtheoof/swappy/commit/445980bbf4702e59113fab506b2e9e36ad931666)), closes [#6](https://github.com/jtheoof/swappy/issues/6)
* **wayland:** initialize done copies to 0 ([65cefc1](https://github.com/jtheoof/swappy/commit/65cefc1da7fed86508301250ffc1b6dbc9fd3692))
* **wayland:** replace g_error by g_warning ([64bfc2b](https://github.com/jtheoof/swappy/commit/64bfc2b3a71ed00d0dc1102501ac85792735833f))
* **window:** quit when delete event is received ([0c5e458](https://github.com/jtheoof/swappy/commit/0c5e458d4c44a2e2e2b4451b4576724aef2a06b0))


* refactor!(wayland): remove wayland code ([204a93e](https://github.com/jtheoof/swappy/commit/204a93eb0f696bc7be8335d46212c6024e3b2c51))
