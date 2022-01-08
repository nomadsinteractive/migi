from migi import Injector, Device


class Session:
    def __init__(self, device: Device, injector: Injector):
        self._device = device
        self._injector = injector

    def detach(self):
        pass

    def terminate(self):
        pass
