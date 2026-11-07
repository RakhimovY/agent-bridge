# Wake phrase matching

This small, device-independent matcher compares sequences of audio features against one or more reference sequences with dynamic time warping. It is used by the experimental [Yandex Station wake detector](../../integrations/yandex-station/wake-word/README.md). The reference files are private, user-specific data, not phrases or model weights shipped by the project.

The matcher does not train a model or require a specific name or language. It does not guarantee accurate recognition for every phrase, voice, device, or room. Different speakers may need their own examples, and short or similar-sounding phrases may produce false activations. Use representative positive and negative audio to assess a particular setup.

The matching unit tests are offline and do not contact a device or account:

```sh
c++ -std=c++17 shared/wake/match_test.cpp -o /tmp/dot-wake-test
/tmp/dot-wake-test
```
