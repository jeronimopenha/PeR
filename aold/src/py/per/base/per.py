from abc import ABC, abstractmethod
from enum import Enum, auto
from typing import List


class EdAlgEnum(Enum):
    ZIG_ZAG: int = auto()
    DEPTH_FIRST_NO_PRIORITY: int = auto()
    DEPTH_FIRST_WITH_PRIORITY: int = auto()


class PeR_Enum(Enum):
    YOTO: int = auto()
    YOTT: int = auto()
    SA: int = auto()


class PeR(ABC):

    @abstractmethod
    def per(self, per_alg: PeR_Enum, parameters: List, n_exec: int = 1):
        pass

    @abstractmethod
    def sa_worker(cls, exec_id: int, report, lock, parameters: List):
        pass

    @abstractmethod
    def yoto_worker(cls, exec_id: int, report, lock, parameters: List):
        pass

    @abstractmethod
    def yott_worker(cls, exec_id: int, report, lock, parameters: List):
        pass
