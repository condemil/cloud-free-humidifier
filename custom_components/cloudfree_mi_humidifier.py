"""Support for Cloudfree MI Humidifiers."""
import json
import logging
from enum import Enum

import voluptuous as vol

from homeassistant.components import mqtt
from homeassistant.components.fan import FanEntity
from homeassistant.const import (
    CONF_DEVICE,
    CONF_NAME,
)
from homeassistant.core import callback
import homeassistant.helpers.config_validation as cv
from homeassistant.helpers.typing import ConfigType, HomeAssistantType

from homeassistant.components.fan import SUPPORT_SET_SPEED
from homeassistant.components.mqtt import (
    CONF_COMMAND_TOPIC,
    CONF_QOS,
    CONF_RETAIN,
    CONF_STATE_TOPIC,
    CONF_UNIQUE_ID,
    MqttAttributes,
    MqttEntityDeviceInfo,
    subscription,
)

_LOGGER = logging.getLogger(__name__)


DEFAULT_NAME = "Cloudfree MI Fan"

PLATFORM_SCHEMA = (
    mqtt.MQTT_RW_PLATFORM_SCHEMA.extend(
        {
            vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
            vol.Optional(CONF_UNIQUE_ID): cv.string,
        }
    )
)


class Speed(Enum):
    Silent = 'silent'
    Medium = 'medium'
    High = 'high'
    Auto = 'auto'


async def async_setup_platform(
    _: HomeAssistantType, config: ConfigType, async_add_entities, __=None
):
    async_add_entities([CloudfreeMiHumidifier(config)])


# pylint: disable=too-many-ancestors
class CloudfreeMiHumidifier(
    MqttAttributes,
    MqttEntityDeviceInfo,
    FanEntity,
):
    def __init__(self, config):
        self._unique_id = 'zhimi.humidifier.cb1-{}'.format('todo')
        self._sub_state = None

        self._topic = None
        self._payload = None
        self._templates = None

        # states
        self._state: bool = False
        self._speed: Speed = Speed.Auto
        self._depth: int = 0
        self._buzzer: bool = False
        self._dry: bool = False
        self._child_lock: bool = False
        self._temperature: float = None
        self._humidity: int = None

        # Load config
        self._setup_from_config(config)

        device_config = config.get(CONF_DEVICE)

        MqttAttributes.__init__(self, config)
        MqttEntityDeviceInfo.__init__(self, device_config, None)

    async def async_added_to_hass(self):
        """Subscribe to MQTT events."""
        await super().async_added_to_hass()
        await self._subscribe_topics()

    def _setup_from_config(self, config):
        """(Re)Setup the entity."""
        self._config = config

        self._topic = {
            CONF_STATE_TOPIC: config.get(CONF_STATE_TOPIC),
            CONF_COMMAND_TOPIC: config.get(CONF_COMMAND_TOPIC),
        }

    async def _subscribe_topics(self):
        @callback
        def state_received(msg):
            values = json.loads(msg.payload)

            if values["state"] == "ON":
                self._state = True
            elif values["state"] == "OFF":
                self._state = False

            self._speed = Speed(values['mode'])
            self._depth = int(values['depth'])
            self._buzzer = bool(values['buzzer'])
            self._dry = bool(values['dry'])
            self._child_lock = bool(values['child_lock'])
            self._temperature = int(values['temp_dec']) / 10
            self._humidity = int(values['humidity'])

            self.async_write_ha_state()

        if self._topic[CONF_STATE_TOPIC] is not None:
            self._sub_state = await subscription.async_subscribe_topics(
                self.hass,
                self._sub_state,
                {
                    "state_topic": {
                        "topic": self._topic[CONF_STATE_TOPIC],
                        "msg_callback": state_received,
                        "qos": self._config[CONF_QOS],
                    }
                },
            )

    async def async_will_remove_from_hass(self):
        """Unsubscribe when removed."""
        self._sub_state = await subscription.async_unsubscribe_topics(
            self.hass, self._sub_state
        )
        await MqttAttributes.async_will_remove_from_hass(self)

    @property
    def supported_features(self):
        """Flag supported features."""
        return SUPPORT_SET_SPEED

    @property
    def should_poll(self):
        return False

    @property
    def is_on(self):
        return self._state

    @property
    def name(self) -> str:
        return self._config[CONF_NAME]

    @property
    def speed_list(self) -> list:
        """Get the list of available speeds."""
        return [e.value for e in Speed]

    @property
    def speed(self):
        """Return the current speed."""
        return self._speed.name

    @property
    def unique_id(self):
        # TODO: unique id
        return self._unique_id

    async def async_turn_on(self, speed: str = None, **kwargs) -> None:
        """Turn on the entity.

        This method is a coroutine.
        """
        mqtt.async_publish(
            self.hass,
            self._topic[CONF_COMMAND_TOPIC],
            self._payload["STATE_ON"],
            self._config[CONF_QOS],
            self._config[CONF_RETAIN],
        )

    async def async_turn_off(self, **kwargs) -> None:
        """Turn off the entity.

        This method is a coroutine.
        """
        mqtt.async_publish(
            self.hass,
            self._topic[CONF_COMMAND_TOPIC],
            self._payload["STATE_OFF"],
            self._config[CONF_QOS],
            self._config[CONF_RETAIN],
        )

    async def async_set_speed(self, speed: str) -> None:
        # TODO: set speed
        mqtt_payload = ''

        mqtt.async_publish(
            self.hass,
            self._topic[CONF_COMMAND_TOPIC],
            mqtt_payload,
            self._config[CONF_QOS],
            self._config[CONF_RETAIN],
        )
